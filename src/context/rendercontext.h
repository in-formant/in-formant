#ifndef MAIN_RENDER_CONTEXT_H
#define MAIN_RENDER_CONTEXT_H

#include <memory>

#include "../gui/qpainterwrapper.h"

namespace Main {

    class RenderView;
    class Config;
    class DataStore;

    class RenderContext {
    public:
        RenderContext(Config *config, DataStore *dataStore);

        void render(QPainterWrapper *painter);
        void setView(RenderView *view);

    private:
        Config *mConfig;
        DataStore *mDataStore;
        RenderView *mSelectedView;
    };

    class RenderView {
    protected:
        virtual void render(QPainterWrapper *painter, Config *config, DataStore *dataStore) {}

        friend void RenderContext::render(QPainterWrapper *painter);
    };

}

#endif // MAIN_RENDER_CONTEXT_H
