#include "pipeline.h"
#include "../../../analysis/filter/filter.h"

#include <iostream>

using namespace Module::App;

Pipeline::Pipeline(Module::Audio::Buffer *captureBuffer,
                Main::DataStore *dataStore, Main::Config *config,
                std::shared_ptr<Analysis::PitchSolver> pitchSolver,
                std::shared_ptr<Analysis::LinpredSolver> linpredSolver,
                std::shared_ptr<Analysis::FormantSolver> formantSolver,
                std::shared_ptr<Analysis::InvglotSolver> invglotSolver)
    : captureBuffer(captureBuffer),
      dataStore(dataStore),
      config(config),
      pitchSolver(pitchSolver),
      linpredSolver(linpredSolver),
      formantSolver(formantSolver),
      invglotSolver(invglotSolver),
      time(0),
      runningThreads(false),
      bufferSpectrogram(16000),
      bufferPitch(16000),
      bufferFormants(16000)
{
}

Pipeline::~Pipeline()
{
    runningThreads = false;
    
    Module::Audio::Buffer::cancelPulls();

    if (threadSpectrogram.joinable())
        threadSpectrogram.join();

    if (threadPitch.joinable())
        threadPitch.join();

    if (threadFormants.joinable())
        threadFormants.join();
}

void Pipeline::callbackSpectrogram()
{
    double fs = captureBuffer->getSampleRate();

    Analysis::RealFFT fft(4096);
    
    rpm::vector<double> m(25.0 * fs / 1000.0);
    
    auto hpsos = Analysis::butterworthHighpass(8, 50.0, fs);

    double dfs = 2 * config->getViewMaxFrequency();
    Module::Audio::Resampler rs(fs, dfs);

    int64_t t = 0;
    int64_t t1 = 0;

    while (runningThreads) {
        dfs = 2 * config->getViewMaxFrequency();
        rs.setOutputRate(dfs);
        
        //m.resize(rs.getRequiredInLength(fft.getInputLength()));
        
        bufferSpectrogram.pull(m.data(), m.size());
        m = Analysis::sosfilter(hpsos, m);

        auto out = rs.process(m.data(), m.size());

        //Eigen::VectorXd gram = Analysis::gammatoneGram(out, dfs, 128, config->getViewMinFrequency());
    
        auto spec = Analysis::fft_n(fft, m);
        Eigen::VectorXd gram = Eigen::Map<Eigen::VectorXd>(spec.data(), spec.size());

        dataStore->beginWrite();
        dataStore->getSpectrogram().insert(t / fs, {
            .amplitudes = gram,
            .duration = (double) out.size() / dfs,
            .minFrequency = (double) 1, //config->getViewMinFrequency(),
            .maxFrequency = (double) config->getViewMaxFrequency(),
        });
        dataStore->endWrite();

        t += m.size();
        t1 += out.size();
    }
}

void Pipeline::callbackPitch()
{
    double fs = captureBuffer->getSampleRate();

    rpm::vector<double> m(30.0 * fs / 1000.0);

    int64_t t = 0;

    while (runningThreads) {
        bufferPitch.pull(m.data(), m.size());

        auto pitchResult = pitchSolver->solve(m.data(), m.size(), fs);

        dataStore->beginWrite();
        if (pitchResult.voiced) {
            dataStore->getPitchTrack().insert(t / fs, pitchResult.pitch);
        }
        else {
            dataStore->getPitchTrack().insert(t / fs, std::nullopt);
        }
        dataStore->endWrite();
        t += m.size();
    }
}

void Pipeline::callbackFormants()
{
    double fs = captureBuffer->getSampleRate();

    rpm::vector<double> m(20.0 * fs / 1000.0);

    double preemphFrequency = 100;
    double preemphFactor = exp(-(2.0 * M_PI * preemphFrequency) / fs);
    rpm::vector<double> w = Analysis::gaussianWindow(m.size(), 2.5);

    double fsDF = 16000;
    Module::Audio::Resampler rsDF(fs, fsDF);

    double fsLPC = 10000;
    Module::Audio::Resampler rsLPC(fs, fsLPC);

    int64_t t = 0;

    while (runningThreads) {
        bufferFormants.pull(m.data(), m.size());

        // Pre-emphasis and windowing.
        for (int i = m.size() - 1; i >= 1; --i) {
            m[i] = w[i] * (m[i] - preemphFactor * m[i - 1]);
        }

        // Resample both regardless of the formant method in use.
        auto mDF  = rsDF.process(m.data(), m.size());
        auto mLPC = rsLPC.process(m.data(), m.size());

        rpm::vector<double> lpc;

        if (auto deepFormantSolver = dynamic_cast<Analysis::Formant::DeepFormants *>(formantSolver.get())) {
            deepFormantSolver->setFrameAudio(mDF.data(), mDF.size(), fsDF);
        }
        else {
            double gain;
            lpc = linpredSolver->solve(mLPC.data(), mLPC.size(), 10, &gain);
        }

        auto formantResult = formantSolver->solve(lpc.data(), lpc.size(), fsLPC);
       
        dataStore->beginWrite();
        for (int i = 0;
                i < std::min<int>(
                    dataStore->getFormantTrackCount(),
                    formantResult.formants.size());
                ++i) {
            dataStore->getFormantTrack(i).insert(t / fs, formantResult.formants[i].frequency);
        }
        for (int i = formantResult.formants.size(); i < dataStore->getFormantTrackCount(); ++i) {
            dataStore->getFormantTrack(i).insert(t / fs, std::nullopt);
        }
        dataStore->endWrite();
        t += m.size();
    }
}

void Pipeline::processAll()
{
    static int blockSize = 384;
    rpm::vector<double> data(blockSize);
    captureBuffer->pull(data.data(), data.size());
    time += blockSize;

    const double fs = (double) captureBuffer->getSampleRate();

    dataStore->setTime((double) time / fs);

    bufferSpectrogram.setSampleRate(fs);
    bufferPitch.setSampleRate(fs);
    bufferFormants.setSampleRate(fs);

    rpm::vector<float> fdata(data.begin(), data.end());
    bufferSpectrogram.push(fdata.data(), data.size());
    bufferPitch.push(fdata.data(), data.size());
    bufferFormants.push(fdata.data(), data.size());

    if (!runningThreads) {
        runningThreads = true;
        threadSpectrogram = std::thread(std::mem_fn(&Pipeline::callbackSpectrogram), this);
        threadPitch = std::thread(std::mem_fn(&Pipeline::callbackPitch), this);
        threadFormants = std::thread(std::mem_fn(&Pipeline::callbackFormants), this);
    }
 
    dataStore->getSoundTrack().insert(time / fs, data);

    // dynamically adjust blockSize to consume all the buffer.
    int bufferLength = captureBuffer->getLength();
    if (bufferLength >= 8192) {
        blockSize += 128;
        std::cout << "Processing too slowly, "
                  << bufferLength << " samples remaining. "
                  << "Adjusted block size to "
                  << blockSize << " samples" << std::endl;
    }
} 
