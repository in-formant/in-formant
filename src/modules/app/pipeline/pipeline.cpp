#include "pipeline.h"

#include <iostream>

using namespace Module::App;

Pipeline::Pipeline(Module::Audio::Buffer *captureBuffer, Main::DataStore *dataStore)
    : captureBuffer(captureBuffer),
      dataStore(dataStore),

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

Pipeline& Pipeline::setPitchSolver(Analysis::PitchSolver *value)
{
    pitchSolver = value;
    return *this;
}

Pipeline& Pipeline::setInvglotSolver(Analysis::InvglotSolver *value)
{
    invglotSolver = value;
    return *this;
}

Pipeline& Pipeline::setLinpredSolver(Analysis::LinpredSolver *value)
{
    linpredSolver = value;
    return *this;
}

Pipeline& Pipeline::setFormantSolver(Analysis::FormantSolver *value)
{
    formantSolver = value;
    return *this;
}

void Pipeline::callbackSpectrogram()
{
    auto& analyzerOpt = dataStore->getSpectrogramAnalyzer();
    auto& coefsOpt = dataStore->getSpectrogramCoefs();

    rpm::vector<double> m(256);
    
    double fs = captureBuffer->getSampleRate();
    double fsDest = fs; //16000;
    gaborator::parameters params(32, 70.0 / fsDest, 440.0 / fsDest);
    analyzerOpt = gaborator::analyzer<double>(params);
    auto& analyzer = *analyzerOpt;
    coefsOpt = gaborator::coefs<double>(analyzer);
    auto& coefs = *coefsOpt;

    Module::Audio::Resampler resampler(fs, fsDest);

    int64_t t = 0;

    while (runningThreads) {
        bufferSpectrogram.pull(m.data(), m.size());

        auto out = resampler.process(m.data(), m.size());

        dataStore->beginWrite();
        analyzer.analyze(out.data(), t, t + out.size(), coefs);
        dataStore->endWrite();
        t += out.size();
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

        if (pitchResult.voiced) {
            dataStore->beginWrite();
            dataStore->getPitchTrack().insert(t / fs, pitchResult.pitch);
            dataStore->endWrite();
        }
        t += m.size();
    }
}

void Pipeline::callbackFormants()
{
    double fs = captureBuffer->getSampleRate();

    rpm::vector<double> m(25.0 * fs / 1000.0);

    double fsDest = 10000;
    Module::Audio::Resampler resampler(fs, fsDest);

    int64_t t = 0;

    while (runningThreads) {
        bufferFormants.pull(m.data(), m.size());

        // Resample even if we're not using LPC method.
        auto mr = resampler.process(m.data(), m.size());

        rpm::vector<double> lpc;

        if (auto deepFormantSolver = dynamic_cast<Analysis::Formant::DeepFormants *>(formantSolver)) {
            deepFormantSolver->setFrameAudio(m.data(), m.size(), fs);
        }
        else {
            double gain;
            lpc = linpredSolver->solve(mr.data(), mr.size(), 10, &gain);
        }

        auto formantResult = formantSolver->solve(lpc.data(), lpc.size(), fsDest);
       
        dataStore->beginWrite();
        for (int i = 0;
                i < std::min<int>(
                    dataStore->getFormantTrackCount(),
                    formantResult.formants.size());
                ++i) {
            dataStore->getFormantTrack(i).insert(t / fs, formantResult.formants[i].frequency);
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
    
    dataStore->setTime((double) time / (double) captureBuffer->getSampleRate());

    bufferSpectrogram.setSampleRate(captureBuffer->getSampleRate());

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
  
    // dynamically adjust blockSize to consume all the buffer.
    int bufferLength = captureBuffer->getLength();
    if (bufferLength >= 8192) {
        blockSize += 128;
        std::cout << "Processing too slowly, "
                  << bufferLength << " samples remaining. "
                  << "Adjusted block size to "
                  << blockSize << " samples" << std::endl;
    }

    /*processStart("prereqs");

    processArc("prereqs", "rs_fft");
    processArc("rs_fft", "fft");

    processArc("prereqs", "rs_2");
   
    processArc("rs_2", "tail_pitch");
    processArc("tail_pitch", "pitch");

    processArc("rs_2", "tail_invglot");
    processArc("tail_invglot", "invglot");

    processArc("rs_2", "tail_linpred");
    processArc("tail_linpred", "preemph_linpred");
    processArc("preemph_linpred", "linpred_spectrum");

    processArc("prereqs", "rs_formant");
    processArc("rs_formant", "tail_formant");
    processArc("tail_formant", "preemph_formant");
    processArc("preemph_formant", "linpred_formant");
    processArc("linpred_formant", "formants");
    
    updateOutputData();*/
}

/*void Pipeline::updateOutputData()
{
    auto ioFFT = nodeIOs["fft"][0]->as<Nodes::IO::AudioSpec>();
    int fftSliceLength = ioFFT->getLength();
    fftSlice.resize(fftSliceLength);
    for (int i = 0; i < fftSliceLength; ++i) {
        fftSlice[i][0] = (fftSampleRate * i) / (2.0 * fftSliceLength);
        fftSlice[i][1] = ioFFT->getConstData()[i];
    }
    
    auto ioLpSpec = nodeIOs["linpred_spectrum"][1]->as<Nodes::IO::AudioSpec>();
    int lpSpecSliceLength = ioLpSpec->getLength();
    lpSpecSlice.resize(lpSpecSliceLength);
    for (int i = 0; i < lpSpecSliceLength; ++i) {
        lpSpecSlice[i][0] = (secondSampleRate * i) / (2.0 * lpSpecSliceLength);
        lpSpecSlice[i][1] = ioLpSpec->getConstData()[i];
    }

    auto ioLpFilter = nodeIOs["linpred_spectrum"][0]->as<Nodes::IO::IIRFilter>();
    int lpSpecLpcLength = ioLpFilter->getFBOrder();
    lpSpecLPC.resize(lpSpecLpcLength);
    for (int i = 0; i < ioLpFilter->getFBOrder(); ++i) {
        lpSpecLPC[i] = ioLpFilter->getFBConstData()[i];
    }

    auto ioPitch = nodeIOs["pitch"][0]->as<Nodes::IO::Frequencies>();
    pitch = (ioPitch->getLength() > 0) ? ioPitch->get(0) : -1.0;

    auto ioFormantF = nodeIOs["formants"][0]->as<Nodes::IO::Frequencies>();
    auto ioFormantB = nodeIOs["formants"][1]->as<Nodes::IO::Frequencies>();
    int formantCount = ioFormantF->getLength();
    formants.resize(formantCount);
    for (int i = 0; i < formantCount; ++i) {
        formants[i] = {
            .frequency = ioFormantF->get(i),
            .bandwidth = ioFormantB->get(i),
        };
    }

    auto ioSound = nodeIOs["tail_invglot"][0]->as<Nodes::IO::AudioTime>();
    sound.assign(ioSound->getConstData(), ioSound->getConstData() + ioSound->getLength());

    auto ioGlot = nodeIOs["invglot"][0]->as<Nodes::IO::AudioTime>();
    glot.assign(ioGlot->getConstData(), ioGlot->getConstData() + ioGlot->getLength());

    auto ioGlotInst = nodeIOs["invglot"][1]->as<Nodes::IO::AudioTime>();
    glotInst.assign(ioGlotInst->getConstData(), ioGlotInst->getConstData() + ioGlotInst->getLength());
}

void Pipeline::createNodes()
{
    nodes["prereqs"]            = std::make_unique<Nodes::Prereqs>(captureBuffer, 50, 0);

    nodes["rs_fft"]             = std::make_unique<Nodes::Resampler>(captureSampleRate, fftSampleRate);
    nodes["fft"]                = std::make_unique<Nodes::Spectrum>(fftSize);
    
    nodes["rs_2"]               = std::make_unique<Nodes::Resampler>(captureSampleRate, secondSampleRate);
    nodes["preemph_linpred"]    = std::make_unique<Nodes::PreEmphasis>();
    nodes["tail_pitch"]         = std::make_unique<Nodes::Tail>(30);
    nodes["pitch"]              = std::make_unique<Nodes::PitchTracker>(pitchSolver);
    nodes["tail_invglot"]       = std::make_unique<Nodes::Tail>(50);
    nodes["invglot"]            = std::make_unique<Nodes::InvGlot>(invglotSolver);
    nodes["tail_linpred"]       = std::make_unique<Nodes::Tail>(15);
    nodes["linpred_spectrum"]   = std::make_unique<Nodes::LinPred>(linpredSolver, lpSpecLpOrder);
    
    nodes["rs_formant"]         = std::make_unique<Nodes::Resampler>(captureSampleRate, formantSampleRate);
    nodes["preemph_formant"]    = std::make_unique<Nodes::PreEmphasis>();
    nodes["tail_formant"]       = std::make_unique<Nodes::Tail>(15);
    nodes["linpred_formant"]    = std::make_unique<Nodes::LinPred>(linpredSolver, formantLpOrder);
    nodes["formants"]           = std::make_unique<Nodes::FormantTracker>(formantSolver);
}*/

