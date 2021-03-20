#include "pipeline.h"
#include "../../../analysis/filter/filter.h"
#include "../../../synthesis/synthesis.h"
#include "../../../context/timings.h"

#include <cctype>
#include <chrono>
#include <iostream>
#include <soxr.h>

using namespace Module::App;

Pipeline::Pipeline(Module::Audio::Buffer *captureBuffer,
                Main::DataStore *dataStore, Main::Config *config,
                std::shared_ptr<Analysis::PitchSolver>& pitchSolver,
                std::shared_ptr<Analysis::LinpredSolver>& linpredSolver,
                std::shared_ptr<Analysis::FormantSolver>& formantSolver,
                std::shared_ptr<Analysis::InvglotSolver>& invglotSolver)
    : mCaptureBuffer(captureBuffer),
      mDataStore(dataStore),
      mConfig(config),
      mPitchSolver(pitchSolver),
      mLinpredSolver(linpredSolver),
      mFormantSolver(formantSolver),
      mInvglotSolver(invglotSolver),
      mTime(0),
      mThreadRunning(false),
      mStopThread(false),
      mBuffer(16000)
{
}

Pipeline::~Pipeline()
{
    mThreadRunning = false;
    mStopThread = true;
    
    Module::Audio::Buffer::cancelPulls();

    if (mProcessingThread.joinable())
        mProcessingThread.join();
}

void Pipeline::callbackProcessing()
{
    rpm::vector<double> block;
    rpm::deque<double> slidingWindow;

    double time = 0;
    mSpectrogramTime = 0;
    mPitchTime = 0;
    mFormantTime = 0;
    mOscilloscopeTime = 0;

    mSpectrogramHighpassSampleRate = 0;
    mSpectrogramHold = 1.0;

    while (mThreadRunning && !mStopThread) {
        const double granularity        = mConfig->getAnalysisGranularity()        / 1000;
        const double pitchWindow        = mConfig->getAnalysisPitchWindow()        / 1000;
        const double formantWindow      = mConfig->getAnalysisFormantWindow()      / 1000;
        const double oscilloscopeWindow = mConfig->getAnalysisOscilloscopeWindow() / 1000;

        block.resize(granularity * mSampleRate);
        mBuffer.pull(block.data(), block.size());
        
        timer_guard timer(timings::update);

        const double longestWindow = std::max(
                {granularity, pitchWindow, formantWindow, oscilloscopeWindow});

        slidingWindow.resize(longestWindow * mSampleRate, 0.0);
        std::rotate(slidingWindow.begin(),
                std::next(slidingWindow.begin(), block.size()),
                slidingWindow.end());
        std::copy(block.begin(), block.end(), std::prev(slidingWindow.end(), block.size()));

        // Process for spectrogram with the same granularity.
        const int overlapSamples = granularity * mSampleRate;
        mSpectrogramTime = time;
        mSpectrogramOverlap.resize(overlapSamples);
        std::copy(std::prev(slidingWindow.end(), overlapSamples), slidingWindow.end(),
                mSpectrogramOverlap.begin());

        processSpectrogram();

        // Process for pitch if needed.
        if (time - mPitchTime >= pitchWindow) {
            const int windowSamples = pitchWindow * mSampleRate;
            mPitchTime = time;
            mPitchData.resize(windowSamples);
            std::copy(std::prev(slidingWindow.end(), windowSamples), slidingWindow.end(),
                    mPitchData.begin());

            processPitch();
        }

        // Process for formants if needed.
        if (time - mFormantTime >= formantWindow) {
            const int windowSamples = formantWindow * mSampleRate;
            mFormantTime = time;
            mFormantData.resize(windowSamples);
            std::copy(std::prev(slidingWindow.end(), windowSamples), slidingWindow.end(),
                    mFormantData.begin());

            processFormants();
        }

        // Process for oscilloscope if needed.
        if (time - mOscilloscopeTime >= oscilloscopeWindow) {
            const int windowSamples = oscilloscopeWindow * mSampleRate;
            mOscilloscopeTime = time;
            mOscilloscopeData.resize(windowSamples);
            std::copy(std::prev(slidingWindow.end(), windowSamples), slidingWindow.end(),
                    mOscilloscopeData.begin());

            processOscilloscope();
        }

        time += granularity;
    }
}

void Pipeline::processSpectrogram()
{
    const double spectrogramWindow  = mConfig->getAnalysisSpectrogramWindow()  / 1000;
    const double fsView = 2 * mConfig->getViewMaxFrequency();

    const int spectrogramSamples = spectrogramWindow * fsView;

    // Resample.
    mSpectrogramResampler.setRate(mSampleRate, fsView);
    auto outOverlap = mSpectrogramResampler.process(mSpectrogramOverlap.data(), mSpectrogramOverlap.size());

    // De-emphasize the very low frequencies.
    if (mSpectrogramHighpassSampleRate != fsView) {
        mSpectrogramHighpass = Analysis::butterworthHighpass(8, 60.0, fsView);
        mSpectrogramHighpassMemory.resize(mSpectrogramHighpass.size(), rpm::vector<double>(2, 0.0));
    }
    outOverlap = Synthesis::sosfilter(mSpectrogramHighpass, outOverlap, mSpectrogramHighpassMemory);

    // Sliding window for the spectrogram.
    mSpectrogramData.resize(spectrogramSamples);
    std::rotate(mSpectrogramData.begin(),
            std::next(mSpectrogramData.begin(), outOverlap.size()),
            mSpectrogramData.end());
    std::copy(outOverlap.begin(), outOverlap.end(), std::prev(mSpectrogramData.end(), outOverlap.size()));

    // Create the FFT calculator if not already made.
    int nfft = mConfig->getViewFFTSize();
    if (!mSpectrogramFFT || mSpectrogramFFT->getInputLength() != nfft) {
        mSpectrogramFFT = std::make_unique<Analysis::RealFFT>(nfft);
    }

    auto fftVector = Analysis::fft_n(*mSpectrogramFFT, mSpectrogramData);
    Eigen::VectorXd spectrum = Eigen::Map<Eigen::VectorXd>(fftVector.data(), fftVector.size());

    double max = spectrum.maxCoeff();
    mSpectrogramHold = max = std::max(0.995 * mSpectrogramHold + 0.005 * max, max);
    spectrum /= max;

    mDataStore->beginWrite();

    mDataStore->getSpectrogram().insert(mSpectrogramTime - mSpectrogramData.size() / mSampleRate,
        {
            .magnitudes = spectrum,
            .sampleRate = fsView,
            .frameDuration = mSpectrogramData.size() / mSampleRate,
        }
    );

    mDataStore->endWrite();
}

void Pipeline::processPitch()
{
    auto pitchResult = mPitchSolver->solve(mPitchData.data(), mPitchData.size(), mSampleRate);
    
    mDataStore->beginWrite();

    if (pitchResult.voiced) {
        mDataStore->getPitchTrack().insert(mPitchTime, pitchResult.pitch);
    }
    else {
        mDataStore->getPitchTrack().insert(mPitchTime, std::nullopt);
    }

    mDataStore->endWrite();
}

void Pipeline::processFormants()
{
    constexpr double preemphFrequency = 100;
    constexpr double fsLPC = 11000;
    constexpr double fs16k = 16000;
    
    const double preemphFactor = exp(-(2.0 * M_PI * preemphFrequency) / mSampleRate);
    
    if (mFormantWindow.size() != mFormantData.size()) {
        mFormantWindow = Analysis::gaussianWindow(mFormantData.size(), 2.5);
    }

    mFormantResamplerLPC.setRate(mSampleRate, fsLPC);
    mFormantResampler16k.setRate(mSampleRate, fs16k);

    // Preemphasis and windowing.
    static double lastPreviousSample = 0.0;
    for (int i = mFormantData.size() - 1; i >= 1; --i) {
        mFormantData[i] = mFormantWindow[i]
                            * (mFormantData[i] - preemphFactor * mFormantData[i - 1]);
    }
    const double m0 = mFormantData[0];
    mFormantData[0] = mFormantWindow[0] * (mFormantData[0] - preemphFactor * lastPreviousSample);
    lastPreviousSample = m0;

    // Resample.
    auto mLPC = mFormantResamplerLPC.process(mFormantData.data(), mFormantData.size());
    auto m16k = mFormantResampler16k.process(mFormantData.data(), mFormantData.size());
    
    rpm::vector<double> lpc;

    if (auto dfSolver = dynamic_cast<Analysis::Formant::DeepFormants *>(mFormantSolver.get())) {
        dfSolver->setFrameAudio(m16k);
        // Pass an empty lpc vector in this case.
    }
    else {
        double gain;
        lpc = mLinpredSolver->solve(mLPC.data(), mLPC.size(), 10, &gain);
    }

    auto formantResult = mFormantSolver->solve(lpc.data(), lpc.size(), fsLPC);

    mDataStore->beginWrite();

    for (int i = 0;
            i < std::min<int>(
                mDataStore->getFormantTrackCount(),
                formantResult.formants.size());
            ++i) {
        const double frequency = formantResult.formants[i].frequency;
        if (std::isnormal(frequency)) {
            mDataStore->getFormantTrack(i).insert(mFormantTime, frequency);
        }
        else {
            mDataStore->getFormantTrack(i).insert(mFormantTime, std::nullopt);
        }
    }
    for (int i = formantResult.formants.size(); i < mDataStore->getFormantTrackCount(); ++i) {
        mDataStore->getFormantTrack(i).insert(mFormantTime, std::nullopt);
    }

    mDataStore->endWrite();
}

void Pipeline::processOscilloscope()
{
    constexpr double fsOsc = 8000;

    mOscilloscopeResampler.setRate(mSampleRate, fsOsc);

    auto out = mOscilloscopeResampler.process(mOscilloscopeData.data(), mOscilloscopeData.size());
    auto invglotResult = mInvglotSolver->solve(out.data(), out.size(), fsOsc);

    mDataStore->beginWrite();
   
    mDataStore->getSoundTrack().insert(mOscilloscopeTime, out);
    mDataStore->getGifTrack().insert(mOscilloscopeTime, invglotResult.glotSig);

    mDataStore->endWrite();
}

void Pipeline::processAll()
{
    const double fs = (double) mCaptureBuffer->getSampleRate();

    static int blockSize = 512;
    rpm::vector<double> data(blockSize);
    mCaptureBuffer->pull(data.data(), data.size());
    mTime = mTime + blockSize / fs;

    mDataStore->setTime(mTime);
    mSampleRate = fs;

    rpm::vector<float> fdata(data.begin(), data.end());
    mBuffer.push(fdata.data(), data.size());

    bool shouldNotBeRunning = false;
    if (mThreadRunning.compare_exchange_strong(shouldNotBeRunning, true)) {
        mProcessingThread = std::thread(std::mem_fn(&Pipeline::callbackProcessing), this);
    }

    // dynamically adjust blockSize to consume all the buffer.
    static int lastBufferLength = 0;
    int bufferLength = mCaptureBuffer->getLength();
    if (blockSize <= 16384 && lastBufferLength - bufferLength >= 8192) {
        blockSize += 128;
        std::cout << "Processing too slowly, "
                  << bufferLength << " samples remaining. "
                  << "Adjusted block size to "
                  << blockSize << " samples" << std::endl;
    }
    else if (blockSize >= 512 && lastBufferLength - bufferLength <= -1024) {
        blockSize -= 128;
        std::cout << "Processing fast enough, "
                  << "adjusted block size to "
                  << blockSize << " samples" << std::endl;
    }
    lastBufferLength = bufferLength;
} 
