#ifndef MAIN_CONTEXT_VIEWS_VIEWS_H
#define MAIN_CONTEXT_VIEWS_VIEWS_H

#include "rpcxx.h"
#include "../rendercontext.h"
#include "../guicontext.h"
#include "../datastore.h"
#include "../config.h"
#include <QThread>
#include <QThreadPool>

namespace Main {
    class AbstractView : public RenderView, public GuiView {
    public:
        virtual ~AbstractView() = default;
    };
}

#include "spectrogram.h"

#endif // MAIN_CONTEXT_VIEWS_VIEWS_H
