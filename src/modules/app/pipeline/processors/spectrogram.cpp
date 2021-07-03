#include "spectrogram.h"

#include "../../../../synthesis/synthesis.h"

using namespace Module::App::Processors;

Spectrogram::Spectrogram(Main::Config *config, Main::DataStore *dataStore)
    : BaseProcessor(0.0,
                    config->getAnalysisGranularity()),
      mConfig(config),
      mDataStore(dataStore),
      mHighpassSampleRate(0),
      mHold(1.0)
{
}

void Spectrogram::processData(const rpm::vector<double>& overlap, double sampleRate)
{
    const double fsView = 2.0 * mConfig->getViewMaxFrequency();
    const int fftSamples = mConfig->getViewFFTSize();

    // Initialize the highpass filter.
    if (mHighpassSampleRate != fsView) {
        mHighpass = Analysis::butterworthHighpass(8, 60.0, fsView);
        mHighpassMemory.resize(mHighpass.size(), rpm::vector<double>(2, 0.0));
    }

    // Resample the input data.
    mResampler.setRate(sampleRate, fsView);
    auto outOverlap = mResampler.process(overlap);
    
    // Apply highpass filter to it.
    outOverlap = Synthesis::sosfilter(mHighpass, outOverlap, mHighpassMemory);

    // Sliding window for the spectrogram.
    mData.resize(fftSamples);
    std::rotate(mData.begin(), std::next(mData.begin(), outOverlap.size()), mData.end());
    std::copy(outOverlap.begin(), outOverlap.end(), std::prev(mData.end(), outOverlap.size()));

    // Create the FFT processor.
    if (!mFFT || mFFT->getInputLength() != fftSamples) {
        mFFT = std::make_shared<Analysis::RealFFT>(fftSamples);
    }

    auto fftVector = Analysis::fft_n(mFFT, mData);

    double max = 0;
    for (const double& x : fftVector) {
        if (x > max)
            max = x;
    }
    mHold = max = std::max(0.995 * mHold + 0.005 * max, max);
    for (double& x : fftVector) {
        x /= max;
    }

    mDataStore->beginWrite();
    
    mDataStore->getSpectrogram().insert(getCenteredTime() - (fftSamples / 2.0) / fsView, {fftVector, fsView});

    mDataStore->endWrite();
}
