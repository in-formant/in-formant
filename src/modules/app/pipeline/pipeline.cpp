#include "pipeline.h"
#include "../../../analysis/filter/filter.h"

#include <iostream>

using namespace Module::App;

Pipeline::Pipeline(Module::Audio::Buffer *captureBuffer, Main::DataStore *dataStore,
                std::shared_ptr<Analysis::PitchSolver> pitchSolver,
                std::shared_ptr<Analysis::LinpredSolver> linpredSolver,
                std::shared_ptr<Analysis::FormantSolver> formantSolver,
                std::shared_ptr<Analysis::InvglotSolver> invglotSolver)
    : captureBuffer(captureBuffer),
      dataStore(dataStore),
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
    auto& coefStore = dataStore->getSpectrogramCoefs();

    rpm::vector<double> m(2048);
    
    double fs = captureBuffer->getSampleRate();

    double dfs = 16000;
    Module::Audio::Resampler rs(fs, dfs);
    gaborator::parameters params(48, 20.0 / dfs, 440.0 / dfs);
    gaborator::analyzer<double> analyzer(params);
    coefStore.push_back({
        .analyzer = std::move(analyzer),
        .coefs = gaborator::coefs(analyzer),
        .fs = dfs,
    });

    /*double dfs2 = 24000;
    Module::Audio::Resampler rs2(fs, dfs2);
    gaborator::parameters params2(32, 400.0 / dfs2, 440.0 / dfs2);
    gaborator::analyzer<double> analyzer2(params2);
    coefStore.push_back({
        .analyzer = std::move(analyzer2),
          .coefs = gaborator::coefs(analyzer2),
        .fs = dfs2,
    });*/

    int64_t t = 0;

    int64_t t1 = 0;
    int64_t t2 = 0;

    while (runningThreads) {
        bufferSpectrogram.pull(m.data(), m.size());

        auto out = rs.process(m.data(), m.size());
        //auto out2 = rs2.process(m.data(), m.size());

        dataStore->beginWrite();
        coefStore[0].analyzer.analyze(out.data(), t1, t1 + out.size(), coefStore[0].coefs);
        //coefStore[1].analyzer.analyze(out2.data(), t2, t2 + out2.size(), coefStore[1].coefs);
        dataStore->endWrite();

        t += m.size();
        t1 += out.size();
        //t2 += out2.size();
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
    rpm::vector<double> w = Analysis::gaussianWindow(m.size(), 0.75);

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
