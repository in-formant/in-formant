#ifndef MAIN_CONTEXT_MANAGER_H
#define MAIN_CONTEXT_MANAGER_H

#include "context.h"
#include "../nodes/nodes.h"

#include <unordered_map>
#include <memory>
#include <vector>

namespace Main {

    using namespace Module;

    class ContextManager;

    struct RenderingContextInfo {
        using CallbackType = std::function<void (RenderingContext)>;

        std::string  name;
        CallbackType renderCallback; 
        CallbackType eventCallback;
    };

    class ContextManager {
    public:
        ContextManager(std::unique_ptr<Context>&& ctx);
        ~ContextManager();

        void initialize();
        void start();
        void terminate();

        void loadSettings();

        void updateRendererTargetSize(RenderingContext& rctx);
        void updateRendererParameters(RenderingContext& rctx);
    
        void createRenderingContexts(const std::initializer_list<std::string>& names);

        void createAudioNodes();
        void createAudioIOs();

        void propagateAudio();
        void processAudioNode(const char *in, const std::string& nodeName);

        void updateNewData();

        void render();
        void renderSpectrogram(RenderingContext& rctx);
        void renderFFTSpectrum(RenderingContext& rctx);
        void renderOscilloscope(RenderingContext& rctx);

        void mainBody();

    private:
        std::unique_ptr<Context> ctx;

        Freetype::FontFile *primaryFont;

        std::unordered_map<std::string, std::unique_ptr<Nodes::Node>>                nodes;
        std::unordered_map<std::string, std::vector<std::unique_ptr<Nodes::NodeIO>>> nodeIOs;
        Nodes::NodeIO **ndi, **ndo;

        std::map<std::string, RenderingContextInfo> renderingContextInfos;
        bool endLoop;

        int analysisDuration;
        int analysisMaxFrequency;

        int viewMinFrequency;
        int viewMaxFrequency;
        int viewMinGain;
        int viewMaxGain;
        Renderer::FrequencyScale viewFrequencyScale;

        int fftLength;
        int fftMaxFrequency;

        float preEmphasisFrequency;
        int linPredOrder;

        int spectrogramCount;

        int numFormantsToRender;
        std::vector<std::array<float, 3>> formantColors;

        int uiFontSize;

        std::deque<std::vector<std::array<float, 2>>>  spectrogramTrack;
        std::deque<float>                              pitchTrack;
        std::deque<std::vector<Analysis::FormantData>> formantTrack;

        std::chrono::microseconds durProcessing;
        std::chrono::microseconds durRendering;
        std::chrono::microseconds durLoop;
    };
    
}

#endif // MAIN_CONTEXT_MANAGER_H
