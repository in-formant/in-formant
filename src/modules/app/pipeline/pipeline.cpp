#include "pipeline.h"
#include "../../../analysis/filter/filter.h"

#include <iostream>

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
      mRunningThreads(false),
      mStopThreads(false),
      mBufferSpectrogram(16000),
      mBufferPitch(16000),
      mBufferFormants(16000)
{
}

Pipeline::~Pipeline()
{
    mRunningThreads = false;
    mStopThreads = true;
    
    Module::Audio::Buffer::cancelPulls();

    if (mThreadSpectrogram.joinable())
        mThreadSpectrogram.join();

    if (mThreadPitch.joinable())
        mThreadPitch.join();

    if (mThreadFormants.joinable())
        mThreadFormants.join();
}

void Pipeline::callbackSpectrogram()
{
    const double fs = mCaptureBuffer->getSampleRate();
    
    constexpr double frameHop = 12.5 / 1000.0;
    rpm::vector<double> m(frameHop * fs);

    double frameDuration;
    rpm::vector<double> slidingWindow;
    
    auto hpsos = Analysis::butterworthHighpass(4, 40.0, fs);
    rpm::vector<rpm::vector<double>> zfhp(hpsos.size(), rpm::vector<double>(2, 0.0));

    double t = 0;

    double maxHold = 1.0;

    while (mRunningThreads && !mStopThreads) {
        double dfs = 2 * mConfig->getViewMaxFrequency();
        mSpectrumResampler.setRate(fs, dfs);
        
        int nfft = mConfig->getViewFFTSize();
        if (!mSpectrumFFT || mSpectrumFFT->getInputLength() != nfft) {
            mSpectrumFFT = std::make_unique<Analysis::RealFFT>(nfft);
            frameDuration = 50.0 / 1000.0;
        }
        
        slidingWindow.resize(frameDuration * fs);

        mBufferSpectrogram.pull(m.data(), m.size());
        auto out = mSpectrumResampler.process(m.data(), m.size());

        // Rotate to the left to make space for the latest chunk of audio.
        std::rotate(slidingWindow.begin(), std::next(slidingWindow.begin(), out.size()), slidingWindow.end());
        std::copy(out.begin(), out.end(), std::prev(slidingWindow.end(), out.size()));

        auto slidingWindowFiltered = Analysis::sosfiltfilt(hpsos, slidingWindow);
        auto fftVector = Analysis::fft_n(*mSpectrumFFT, slidingWindowFiltered); 
        Eigen::VectorXd spectrum = Eigen::Map<Eigen::VectorXd>(fftVector.data(), fftVector.size());

        double max = spectrum.maxCoeff();
        maxHold = max = std::max(0.995 * maxHold + 0.005 * max, max);
        spectrum /= max;

        mDataStore->beginWrite();
        mDataStore->getSpectrogram().insert(t - frameDuration - mSpectrumResampler.getDelay() / dfs, {
            .magnitudes = spectrum,
            .sampleRate = dfs,
            .frameDuration = frameDuration,
        });
        mDataStore->endWrite();

        t += m.size() / fs;
    }
}

void Pipeline::callbackPitch()
{
    const double fs = mCaptureBuffer->getSampleRate();

    rpm::vector<double> m(30.0 * fs / 1000.0);

    double t = 0;

    while (mRunningThreads && !mStopThreads) {
        mBufferPitch.pull(m.data(), m.size());

        auto pitchResult = mPitchSolver->solve(m.data(), m.size(), fs);

        mDataStore->beginWrite();
        if (pitchResult.voiced) {
            mDataStore->getPitchTrack().insert(t, pitchResult.pitch);
        }
        else {
            mDataStore->getPitchTrack().insert(t, std::nullopt);
        }
        mDataStore->endWrite();
        t += m.size() / fs;
    }
}

void Pipeline::callbackFormants()
{
    double fs = mCaptureBuffer->getSampleRate();

    rpm::vector<double> m(20.0 * fs / 1000.0);

    double preemphFrequency = 100;
    double preemphFactor = exp(-(2.0 * M_PI * preemphFrequency) / fs);
    rpm::vector<double> w = Analysis::gaussianWindow(m.size(), 2.5);

    double fsDF = 16000;
    Module::Audio::Resampler rsDF(fs, fsDF);

    double fsLPC = 11000;
    Module::Audio::Resampler rsLPC(fs, fsLPC);

    double t = 0;

    while (mRunningThreads && !mStopThreads) {
        mBufferFormants.pull(m.data(), m.size());

        // Pre-emphasis and windowing.
        for (int i = m.size() - 1; i >= 1; --i) {
            m[i] = w[i] * (m[i] - preemphFactor * m[i - 1]);
        }

        // Resample both regardless of the formant method in use.
        auto mDF  = rsDF.process(m.data(), m.size());
        auto mLPC = rsLPC.process(m.data(), m.size());

        rpm::vector<double> lpc;
      
        double delay;
        if (auto deepFormantSolver = dynamic_cast<Analysis::Formant::DeepFormants *>(mFormantSolver.get())) {
            deepFormantSolver->setFrameAudio(mDF);
            delay = rsDF.getDelay() / fsDF;
        }
        else {
            double gain;
            lpc = mLinpredSolver->solve(mLPC.data(), mLPC.size(), 10, &gain);
            delay = rsLPC.getDelay() / fsLPC;
        }

        auto formantResult = mFormantSolver->solve(lpc.data(), lpc.size(), fsLPC);
       
        mDataStore->beginWrite();
        for (int i = 0;
                i < std::min<int>(
                    mDataStore->getFormantTrackCount(),
                    formantResult.formants.size());
                ++i) {
            const double freq = formantResult.formants[i].frequency;
            if (std::isfinite(freq)) {
                mDataStore->getFormantTrack(i).insert(t - delay, freq);
            }
            else {
                mDataStore->getFormantTrack(i).insert(t - delay, std::nullopt);
            }
        }
        for (int i = formantResult.formants.size(); i < mDataStore->getFormantTrackCount(); ++i) {
            mDataStore->getFormantTrack(i).insert(t - delay, std::nullopt);
        }
        mDataStore->endWrite();
        t += m.size() / fs;
    }
}

void Pipeline::processAll()
{
    const double fs = (double) mCaptureBuffer->getSampleRate();

    static int blockSize = 512;
    rpm::vector<double> data(blockSize);
    mCaptureBuffer->pull(data.data(), data.size());
    mTime = mTime + blockSize / fs;

    mDataStore->setTime(mTime);

    mBufferSpectrogram.setSampleRate(fs);
    mBufferPitch.setSampleRate(fs);
    mBufferFormants.setSampleRate(fs);

    rpm::vector<float> fdata(data.begin(), data.end());
    mBufferSpectrogram.push(fdata.data(), data.size());
    mBufferPitch.push(fdata.data(), data.size());
    mBufferFormants.push(fdata.data(), data.size());

    bool shouldNotBeRunning = false;
    if (mRunningThreads.compare_exchange_strong(shouldNotBeRunning, true)) {
        mThreadSpectrogram = std::thread(std::mem_fn(&Pipeline::callbackSpectrogram), this);
        mThreadPitch = std::thread(std::mem_fn(&Pipeline::callbackPitch), this);
        mThreadFormants = std::thread(std::mem_fn(&Pipeline::callbackFormants), this);
    }
    
    mDataStore->getSoundTrack().insert(mTime, data);

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
