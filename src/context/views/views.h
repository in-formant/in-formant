#ifndef MAIN_CONTEXT_VIEWS_VIEWS_H
#define MAIN_CONTEXT_VIEWS_VIEWS_H

#include "../rendercontext.h"
#include "../guicontext.h"
#include "../datastore.h"
#include "../config.h"
#include "../../modules/renderer/base/base.h"
#include "../../modules/freetype/freetype.h"

namespace Main {

    class AbstractView : public RenderView, public GuiView {
    };

    namespace View {

        using namespace Module::Renderer;
        using namespace Module::Freetype;

        class Spectrogram : public AbstractView {
        protected:
            void render(RendererBase *renderer, FontProvider *fp, Config *config, DataStore *dataStore) override;

        private:
            SpectrogramRenderData mSpectrumRender;
            std::vector<FormantTrackRenderData> mFormantTracksRender;
            FrequencyTrackRenderData mPitchTrackRender;
        };
    }
}

#endif // MAIN_CONTEXT_VIEWS_VIEWS_H
