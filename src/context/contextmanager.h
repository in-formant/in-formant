#ifndef MAIN_CONTEXT_MANAGER_H
#define MAIN_CONTEXT_MANAGER_H

#include "context.h"
#include "../nodes/nodes.h"
#include "../modules/app/app.h"

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

        void initialize();
        void start();
        void terminate();

#if defined(ANDROID) || defined(__ANDROID__)
        void selectView(const std::string& name);
#endif

    private:
        void loadSettings();

        void updateRendererTargetSize(RenderingContext& rctx);
        void updateRendererParameters(RenderingContext& rctx);
        void updateAllRendererParameters();
    
        void createRenderingContexts(const std::initializer_list<RenderingContextInfo>& infos);

        void initSettingsUI();
        void initSynthUI();

        void updateNodeParameters();
        void updateWithNextFrame();

        void scrollSpectrogram(RenderingContext& rctx);

        void renderSpectrogram(RenderingContext& rctx);
        void renderFFTSpectrum(RenderingContext& rctx);
        void renderOscilloscope(RenderingContext& rctx);
        void renderSettings(RenderingContext& rctx);

        void eventSpectrogram(RenderingContext& rctx);
        void eventFFTSpectrum(RenderingContext& rctx);
        void eventOscilloscope(RenderingContext& rctx);
        void eventSettings(RenderingContext& rctx);

#if ! ( defined(__EMSCRIPTEN__) || defined(ANDROID) || defined(__ANDROID__) )
        void renderSynth(RenderingContext& rctx);
        void eventSynth(RenderingContext& rctx);
#endif

#if defined(ANDROID) || defined(__ANDROID__)
        void initAndroidUI();
        void renderAndroidCommonBefore(RenderingContext& rctx);
        void renderAndroidCommonAfter(RenderingContext& rctx);
        void eventAndroidCommon(RenderingContext& rctx);
#endif

        void mainBody(bool processEvents = true);

#ifdef __EMSCRIPTEN__
        void changeModuleCanvas(const std::string& id);
        void saveModuleCtx(const std::string& id);
#endif

        std::unique_ptr<Context> ctx;

        Freetype::FontFile *primaryFont;

        App::Pipeline pipeline;
        App::Synthesizer synth;

        std::map<std::string, RenderingContextInfo> renderingContextInfos;
        bool endLoop;
        bool isPaused;
        bool displayLpSpec;
        bool useFrameCursor;
        bool isNoiseOn;
        bool displayFormantTracks;
        bool displayLegends;

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
        int pitchAndLpSampleRate;
        int linPredOrder;

        int spectrogramCount;

        int numFormantsToRender;
        std::vector<std::array<float, 3>> formantColors;

        int uiFontSize;

        float specMX, specMY;

        std::vector<SettingsUIField> mSettingFields;
        std::vector<SettingsUIField> mSynthFields;

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
        float min, max;
        std::function<float ()>       value;
        std::function<void (float)>   update;
        std::function<std::string ()> barText;
        int x, y, w, h;
        bool isFocused;
    };
    
}

#define FONT(font, size, rctx) ((font)->with((size), (rctx).renderer->getContextNumber(), (rctx).target.get()))

#endif // MAIN_CONTEXT_MANAGER_H
