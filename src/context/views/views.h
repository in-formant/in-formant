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

    namespace View {

        class Spectrogram : public QObject, public AbstractView {
            Q_OBJECT
       
        public:
            Spectrogram();
            virtual ~Spectrogram();

        protected:
            void render(QPainterWrapper *painter, Config *config, DataStore *dataStore) override;
        };

    }
}

#endif // MAIN_CONTEXT_VIEWS_VIEWS_H
