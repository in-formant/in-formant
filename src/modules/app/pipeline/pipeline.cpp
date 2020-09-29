#include "pipeline.h"

using namespace Module::App;

Pipeline::Pipeline(Module::Audio::Buffer *captureBuffer)
    : ndi(nullptr), ndo(nullptr),
      wasInitializedAtLeastOnce(false),
      captureBuffer(captureBuffer)
{
}

Pipeline::~Pipeline()
{
    if (ndi) delete[] ndi;
    if (ndo) delete[] ndo;
}

void Pipeline::initialize()
{
    captureSampleRate = captureBuffer->getSampleRate();
    createNodes();
    createIOs();
    wasInitializedAtLeastOnce = true;
}

Pipeline& Pipeline::setPitchSolver(Analysis::PitchSolver *value)
{
    pitchSolver = value;
    if (wasInitializedAtLeastOnce) {
        nodes["pitch"] = std::make_unique<Nodes::PitchTracker>(pitchSolver);
    }
    return *this;
}

Pipeline& Pipeline::setInvglotSolver(Analysis::InvglotSolver *value)
{
    invglotSolver = value;
    if (wasInitializedAtLeastOnce) {
        nodes["invglot"] = std::make_unique<Nodes::InvGlot>(invglotSolver);
    }
    return *this;
}

Pipeline& Pipeline::setLinpredSolver(Analysis::LinpredSolver *value)
{
    linpredSolver = value;
    if (wasInitializedAtLeastOnce) {
        nodes["linpred_spectrum"] = std::make_unique<Nodes::LinPred>(linpredSolver, lpSpecLpOrder);
        nodes["linpred_formant"]  = std::make_unique<Nodes::LinPred>(linpredSolver, formantLpOrder);
    }
    return *this;
}

Pipeline& Pipeline::setFormantSolver(Analysis::FormantSolver *value)
{
    formantSolver = value;
    if (wasInitializedAtLeastOnce) {
        nodes["formants"] = std::make_unique<Nodes::FormantTracker>(formantSolver);
    }
    return *this;
}

Pipeline& Pipeline::setAnalysisDuration(millis value)
{
    analysisDuration = value;
    if (wasInitializedAtLeastOnce) {
        nodes["prereqs"]->as<Nodes::Prereqs>()->setOutputDuration(value.count());
        nodes["tail_2"]->as<Nodes::Tail>()->setOutputDuration(value.count());
        nodes["tail_formant"]->as<Nodes::Tail>()->setOutputDuration(value.count());
    }
    return *this;
}

Pipeline& Pipeline::setFFTSampleRate(float value)
{
    fftSampleRate = value;
    if (wasInitializedAtLeastOnce) {
        nodes["rs_fft"]->as<Nodes::Resampler>()->setOutputSampleRate(value);
    }
    return *this;
}

Pipeline& Pipeline::setFFTSize(int value)
{
    fftSize = value;
    if (wasInitializedAtLeastOnce) {
        nodes["fft"]->as<Nodes::Spectrum>()->setFFTLength(value);
    }
    return *this;
}

Pipeline& Pipeline::setPreEmphasisFrequency(float value)
{
    preEmphasisFrequency = value;
    if (wasInitializedAtLeastOnce) {
        nodes["preemph_2"]->as<Nodes::PreEmphasis>()->setFrequency(value);
        nodes["preemph_formant"]->as<Nodes::PreEmphasis>()->setFrequency(value);
    }
    return *this;
}

Pipeline& Pipeline::setPitchAndLpSpectrumSampleRate(float value)
{
    secondSampleRate = value;
    if (wasInitializedAtLeastOnce) {
        nodes["rs_2"]->as<Nodes::Resampler>()->setOutputSampleRate(value);
    }
    setLpSpectrumLpOrder(2 * secondSampleRate / 2000 + 4);
    return *this;
}

Pipeline& Pipeline::setLpSpectrumLpOrder(int value)
{
    lpSpecLpOrder = value;
    if (wasInitializedAtLeastOnce) {
        nodes["linpred_spectrum"]->as<Nodes::LinPred>()->setOrder(value);
    }
    return *this;
}

Pipeline& Pipeline::setFormantSampleRate(float value)
{
    formantSampleRate = value;
    if (wasInitializedAtLeastOnce) {
        nodes["rs_formant"]->as<Nodes::Resampler>()->setOutputSampleRate(value);
    }
    return *this;
}

Pipeline& Pipeline::setFormantLpOrder(int value)
{
    formantLpOrder = value;
    if (wasInitializedAtLeastOnce) {
        nodes["linpred_formant"]->as<Nodes::LinPred>()->setOrder(value);
    }
    return *this;
}

const std::vector<std::array<float, 2>>& Pipeline::getFFTSlice() const
{
    return fftSlice;
}

const std::vector<std::array<float, 2>>& Pipeline::getLpSpectrumSlice() const
{
    return lpSpecSlice;
}

const std::vector<Analysis::FormantData>& Pipeline::getFormants() const
{
    return formants;
}

const float Pipeline::getPitch() const
{
    return pitch;
}

const std::vector<float>& Pipeline::getSound() const
{
    return sound;
}

const std::vector<float>& Pipeline::getGlottalFlow() const
{
    return glot;
}

void Pipeline::processAll()
{
    updatePrereqsForFFT();

    processStart("prereqs");

    processArc("prereqs", "rs_fft");
    processArc("rs_fft", "fft");

    processArc("prereqs", "rs_2");
    processArc("rs_2", "tail_2");
    processArc("tail_2", "pitch");
    processArc("tail_2", "invglot");
    processArc("rs_2", "preemph_2");
    processArc("preemph_2", "tail_2");
    processArc("tail_2", "linpred_spectrum");

    processArc("prereqs", "rs_formant");
    processArc("rs_formant", "preemph_formant");
    processArc("preemph_formant", "tail_formant");
    processArc("tail_formant", "linpred_formant");
    processArc("linpred_formant", "formants");
    
    updateOutputData();
}

void Pipeline::processStart(const std::string& nodeName)
{
    Nodes::NodeIO **outs = Nodes::unpack(nodeIOs[nodeName], &ndo);
    nodes[nodeName]->process(nullptr, outs);
}

void Pipeline::processArc(const std::string& input, const std::string& output)
{
    const Nodes::NodeIO **ins = const_cast<decltype(ins)>(Nodes::unpack(nodeIOs[input], &ndi));
    Nodes::NodeIO **outs = Nodes::unpack(nodeIOs[output], &ndo);
    nodes[output]->process(ins, outs);
}

void Pipeline::updatePrereqsForFFT()
{
    int fftInLength = nodes["rs_fft"]->as<Nodes::Resampler>()->getRequiredInputLength(fftSize);
    nodes["prereqs"]->as<Nodes::Prereqs>()->setMinimumOutputLength(fftInLength);
}

void Pipeline::updateOutputData()
{
    auto ioFFT = nodeIOs["fft"][0]->as<Nodes::IO::AudioSpec>();
    int fftSliceLength = ioFFT->getLength();
    fftSlice.resize(fftSliceLength);
    for (int i = 0; i < fftSliceLength; ++i) {
        fftSlice[i][0] = (fftSampleRate * i) / (2.0f * fftSliceLength);
        fftSlice[i][1] = ioFFT->getConstData()[i];
    }
    
    auto ioLpSpec = nodeIOs["linpred_spectrum"][1]->as<Nodes::IO::AudioSpec>();
    int lpSpecSliceLength = ioLpSpec->getLength();
    lpSpecSlice.resize(lpSpecSliceLength);
    for (int i = 0; i < lpSpecSliceLength; ++i) {
        lpSpecSlice[i][0] = (secondSampleRate * i) / (2.0f * lpSpecSliceLength);
        lpSpecSlice[i][1] = ioLpSpec->getConstData()[i];
    }

    auto ioPitch = nodeIOs["pitch"][0]->as<Nodes::IO::Frequencies>();
    pitch = (ioPitch->getLength() > 0) ? ioPitch->get(0) : -1.0f;

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

    auto ioSound = nodeIOs["tail_2"][0]->as<Nodes::IO::AudioTime>();
    sound.assign(ioSound->getConstData(), ioSound->getConstData() + ioSound->getLength());

    auto ioGlot = nodeIOs["invglot"][0]->as<Nodes::IO::AudioTime>();
    glot.assign(ioGlot->getConstData(), ioGlot->getConstData() + ioGlot->getLength());
}

void Pipeline::createNodes()
{
    nodes["prereqs"]            = std::make_unique<Nodes::Prereqs>(captureBuffer, analysisDuration.count(), 0);

    nodes["rs_fft"]             = std::make_unique<Nodes::Resampler>(captureSampleRate, fftSampleRate);
    nodes["fft"]                = std::make_unique<Nodes::Spectrum>(fftSize);
    
    nodes["rs_2"]               = std::make_unique<Nodes::Resampler>(captureSampleRate, secondSampleRate);
    nodes["preemph_2"]          = std::make_unique<Nodes::PreEmphasis>(preEmphasisFrequency);
    nodes["tail_2"]             = std::make_unique<Nodes::Tail>(analysisDuration.count());
    nodes["pitch"]              = std::make_unique<Nodes::PitchTracker>(pitchSolver);
    nodes["invglot"]            = std::make_unique<Nodes::InvGlot>(invglotSolver);
    nodes["linpred_spectrum"]   = std::make_unique<Nodes::LinPred>(linpredSolver, lpSpecLpOrder);
    
    nodes["rs_formant"]         = std::make_unique<Nodes::Resampler>(captureSampleRate, formantSampleRate);
    nodes["preemph_formant"]    = std::make_unique<Nodes::PreEmphasis>(preEmphasisFrequency);
    nodes["tail_formant"]       = std::make_unique<Nodes::Tail>(analysisDuration.count());
    nodes["linpred_formant"]    = std::make_unique<Nodes::LinPred>(linpredSolver, formantLpOrder);
    nodes["formants"]           = std::make_unique<Nodes::FormantTracker>(formantSolver);
}

void Pipeline::createIOs()
{
    for (const auto& [name, node] : nodes) {
        std::vector<std::unique_ptr<Nodes::NodeIO>> ioVector;
        for (const auto type : node->getOutputTypes()) {
            ioVector.push_back(Nodes::makeNodeIO(type));
        }
        nodeIOs[name] = std::move(ioVector);
    }
}
