#ifndef MAIN_CONTEXT_VIEWS_SPECTROGRAM_H
#define MAIN_CONTEXT_VIEWS_SPECTROGRAM_H

#include "views.h"

namespace Main::View {

    class Spectrogram : public QObject, public AbstractView {
        Q_OBJECT
   
    public:
        Spectrogram();
        virtual ~Spectrogram();

    protected:
        void render(QPainterWrapper *painter, Config *config, DataStore *dataStore) override;
    };

}

#endif
