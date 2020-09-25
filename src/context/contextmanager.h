#ifndef MAIN_CONTEXT_MANAGER_H
#define MAIN_CONTEXT_MANAGER_H

#include "context.h"
#include "../nodes/nodes.h"

#include <unordered_map>
#include <memory>
#include <vector>

namespace Main {

    using namespace Module;

    struct RenderingContextInfo;
    struct SettingsUIField;

    class ContextManager {
    public:
        ContextManager(std::unique_ptr<Context>&& ctx);
        ~ContextManager();

        void initialize();
        void start();
        void terminate();

#if defined(ANDROID) || defined(__ANDROID__)
        void selectView(const std::string& name);
#endif

        void generateAudio(float *x, int n);

    private:
        void loadSettings();

        void updateRendererTargetSize(RenderingContext& rctx);
        void updateRendererParameters(RenderingContext& rctx);
        void updateAllRendererParameters();
    
        void createRenderingContexts(const std::initializer_list<RenderingContextInfo>& infos);

        void createAudioNodes();
        void createAudioIOs();
        void updateNodeParameters();

        void propagateAudio();
        void processAudioNode(const char *in, const std::string& nodeName);

        void updateNewData();

        void initSettingsUI();

        void renderSpectrogram(RenderingContext& rctx);
        void renderFFTSpectrum(RenderingContext& rctx);
        void renderOscilloscope(RenderingContext& rctx);
        void renderSettings(RenderingContext& rctx);

        void eventSpectrogram(RenderingContext& rctx);
        void eventFFTSpectrum(RenderingContext& rctx);
        void eventOscilloscope(RenderingContext& rctx);
        void eventSettings(RenderingContext& rctx);

#if defined(ANDROID) || defined(__ANDROID__)
        void renderAndroidCommon(RenderingContext& rctx);
        void eventAndroidCommon(RenderingContext& rctx);
#endif

        void mainBody(bool processEvents = true);

#ifdef __EMSCRIPTEN__
        void changeModuleCanvas(const std::string& id);
        void saveModuleCtx(const std::string& id);
#endif

        std::unique_ptr<Context> ctx;

        Freetype::FontFile *primaryFont;

        std::unordered_map<std::string, std::unique_ptr<Nodes::Node>>                nodes;
        std::unordered_map<std::string, std::vector<std::unique_ptr<Nodes::NodeIO>>> nodeIOs;
        Nodes::NodeIO **ndi, **ndo;

        std::map<std::string, RenderingContextInfo> renderingContextInfos;
        bool endLoop;
        bool isPaused;
        bool displayLpSpec;
        bool useFrameCursor;
        bool isNoiseOn;

#if defined(ANDROID) || defined(__ANDROID__)
        std::string selectedViewName;
#endif

        float outputGain;
            
        int analysisDuration;
        int analysisMaxFrequency;

        int viewMinFrequency;
        int viewMaxFrequency;
        int viewMinGain;
        int viewMaxGain;
        Renderer::FrequencyScale viewFrequencyScale;

        int fftLength;
        int fftMaxFrequency;

        int preEmphasisFrequency;
        int linPredOrder;

        int spectrogramCount;

        int numFormantsToRender;
        std::vector<std::array<float, 3>> formantColors;

        int uiFontSize;

        float specMX, specMY;

        std::vector<SettingsUIField> mSettingFields;

        std::deque<std::vector<std::array<float, 2>>>  spectrogramTrack;
        std::deque<std::vector<std::array<float, 2>>>  lpSpecTrack;
        std::deque<float>                              pitchTrack;
        std::deque<std::vector<Analysis::FormantData>> formantTrack;
        std::deque<std::vector<float>>                 soundTrack;
        std::deque<std::vector<float>>                 glotTrack;

        std::chrono::microseconds durProcessing;
        std::chrono::microseconds durRendering;
        std::chrono::microseconds durLoop;
    };

    struct RenderingContextInfo {
        using CallbackType = void (ContextManager::*)(RenderingContext&);

        std::string  name;
        CallbackType renderCallback; 
        CallbackType eventCallback;

#ifdef __EMSCRIPTEN__
        std::string  canvasId;
#endif
    };
    
    struct SettingsUIField {
        std::string labelText;
        int ContextManager::*field;

        int x, y, w, h;
        bool isFocused;
    };
    
}

#endif // MAIN_CONTEXT_MANAGER_H
