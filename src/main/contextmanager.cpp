#include "contextmanager.h"

using namespace Main;

ContextManager::ContextManager(std::unique_ptr<Context>& ctx)
    : ctx(std::move(ctx)),
      ndi(nullptr), ndo(nullptr)
{
}

ContextManager::~ContextManager()
{
    if (ndi != nullptr) delete[] ndi;
    if (ndo != nullptr) delete[] ndo;
}

void ContextManager::initialize()
{
    ctx->audio->initialize();
    ctx->audio->refreshDevices();
    
    ctx->target->initialize();
    ctx->target->setTitle("Speech analysis");
    ctx->target->setSize(854, 480);
}

void ContextManager::start()
{
    ctx->audio->openPlaybackStream(nullptr);
    ctx->audio->startPlaybackStream();
    
    ctx->audio->openCaptureStream(nullptr);
    ctx->audio->startCaptureStream();

    ctx->target->create();

    ctx->freetypeInstance = std::make_unique<Freetype::FTInstance>(ctx->target.get());
    primaryFont = & ctx->freetypeInstance->font("Montserrat.otf");

    ctx->renderer->initialize();
    updateRendererTargetSize();
    loadRenderingParameters(ctx->renderer->getParameters());

    while (!ctx->target->shouldQuit()) {
        mainBody();
    }
}

void ContextManager::loadSettings()
{
    analysisDuration = 25;
    analysisMaxFrequency = 3700;

    viewMinFrequency = 20;
    viewMaxFrequency = 8000;
    viewMinGain = -45;
    viewMaxGain = +10;
    viewFrequencyScale = Renderer::FrequencyScale::Mel;

    fftLength = 2048;
    fftMaxFrequency = viewMaxFrequency;

    preEmphasisFrequency = 50.0f;

    spectrogramCount = 400;

    numFormantsToRender = 3;
    formantColors = {
        {0.0f,  1.0f,  0.0f},
        {0.86f, 0.78f, 0.24f},
        {1.0f,  0.71f, 0.76f},
    };

    spectrogramTrack.resize(spectrogramCount);
    pitchTrack.resize(spectrogramCount);
    formantTrack.resize(spectrogramCount);
}

void ContextManager::updateRendererTargetSize()
{
    int w, h;

    ctx->target->getSizeForRenderer(&w, &h);
    ctx->renderer->setDrawableSize(w, h);

    ctx->target->getSize(&w, &h);
    ctx->renderer->setWindowSize(w, h);
}

void ContextManager::loadRenderingParameters(Renderer::Parameters *p)
{
    p->setMinFrequency(viewMinFrequency);
    p->setMaxFrequency(viewMaxFrequency);
    p->setMinGain(viewMinGain);
    p->setMaxGain(viewMaxGain);
    p->setFrequencyScale(viewFrequencyScale);
}

void ContextManager::createAudioNodes()
{
    int captureSampleRate = ctx->captureBuffer->getSampleRate();

    nodes["prereqs"] = std::make_unique<Nodes::Prereqs>(ctx->captureBuffer.get(), analysisDuration, 0);

    nodes["rs_spec"] = std::make_unique<Nodes::Resampler>(captureSampleRate, 2 * fftMaxFrequency);
    nodes["spec"] = std::make_unique<Nodes::Spectrum>(fftLength);

    nodes["tail"] = std::make_unique<Nodes::Tail>(analysisDuration);
   
    nodes["pitch"] = std::make_unique<Nodes::PitchTracker>(ctx->pitchSolver.get());
    
    nodes["rs"] = std::make_unique<Nodes::Resampler>(captureSampleRate, 2 * analysisMaxFrequency);
    nodes["preemph"] = std::make_unique<Nodes::PreEmphasis>(preEmphasisFrequency);
    nodes["linpred"] = std::make_unique<Nodes::LinPred>(ctx->linpredSolver.get(), linPredOrder);
    nodes["formant"] = std::make_unique<Nodes::FormantTracker>(ctx->formantSolver.get());
}

void ContextManager::createAudioIOs()
{
    nodeIOs["prereqs"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["rs_spec"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["spec"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioSpec);
    nodeIOs["tail"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["pitch"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeFrequencies);
    nodeIOs["rs"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["preemph"] = Nodes::makeNodeIO(1, Nodes::kNodeIoTypeAudioTime);
    nodeIOs["linpred"] = Nodes::makeNodeIO(2, Nodes::kNodeIoTypeIIRFilter, Nodes::kNodeIoTypeAudioSpec);
    nodeIOs["formant"] = Nodes::makeNodeIO(2, Nodes::kNodeIoTypeFrequencies, Nodes::kNodeIoTypeFrequencies);
}

void ContextManager::propagateAudio()
{
    nodes["prereqs"]->as<Nodes::Prereqs>->setMinimumOutputLength(
            nodes["rs_spec"]->as<Nodes::Resampler>->getRequiredInputLength(
                nodes["spec"]->as<Nodes::Spectrum>->getFFTLength()));

    processAudioNode("prereqs", nullptr, "prereqs");

    processAudioNode("rs_spec", "prereqs", "rs_spec");
    processAudioNode("spec", "rs_spec", "spec");
    
    processAudioNode("tail", "prereqs", "tail");
    
    processAudioNode("pitch", "tail", "pitch");

    processAudioNode("rs", "tail", "rs");
    processAudioNode("preemph", "rs", "preemph");
    processAudioNode("linpred", "preemph", "linpred");
    processAudioNode("formant", "linpred", "formant");
}

void ContextManager::processAudioNode(const std::string& nodeName, const char *in, const char *out)
{
    const Nodes::NodeIO **ins = nullptr;    
    if (in != nullptr) {
        ins = const_cast<decltype(ins)>(Nodes::unpack(nodeIOs[in], &ndi));
    }

    Nodes::NodeIO **outs = nullptr;
    if (out != nullptr) {
        outs = Nodes::unpack(nodeIOs[out], &ndo);
    }

    nodes[nodeName]->process(ins, outs);
}

void ContextManager::mainBody()
{
    
}
