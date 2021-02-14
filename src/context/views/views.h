#ifndef MAIN_CONTEXT_VIEWS_VIEWS_H
#define MAIN_CONTEXT_VIEWS_VIEWS_H

#include "rpcxx.h"
#include "../rendercontext.h"
#include "../guicontext.h"
#include "../datastore.h"
#include "../config.h"
#include <QThread>

namespace Main {

    class AbstractView : public RenderView, public GuiView {
    public:
        virtual ~AbstractView() = default;
    };

    namespace View {

        class SpectrogramWorker : public QObject {
            Q_OBJECT

        public:
            SpectrogramWorker();
            virtual ~SpectrogramWorker() {}

            static bool queued();

        public slots:
            void render(
                    double timeStart,
                    double timeEnd,
                    const rpm::vector<double>& amplitudes,
                    int w, int h, int vw, int vh,
                    QPainterWrapper::FrequencyScale scale,
                    double minFrequency, double maxFrequency,
                    double minGain, double maxGain);
        
        signals:
            void rendered(QImage image, double timeStart, double timeEnd);

        private:
            static std::atomic_bool mQueued;
        };

        class Spectrogram : public QObject, public AbstractView {
            Q_OBJECT
       
        public:
            Spectrogram();
            virtual ~Spectrogram();

        protected:
            void render(QPainterWrapper *painter, Config *config, DataStore *dataStore) override;

        signals:
            void renderSpectrogram(
                    double timeStart,
                    double timeEnd,
                    const rpm::vector<double>& amplitudes,
                    int w, int h, int vw, int vh,
                    QPainterWrapper::FrequencyScale scale,
                    double minFrequency, double maxFrequency,
                    double minGain, double maxGain);

        public slots:
            void spectrogramRendered(QImage image, double timeStart, double timeEnd);
        
        private:
            QThread mRenderThread;

            QImage mImage;
            double mTime;
        };

    }
}

#endif // MAIN_CONTEXT_VIEWS_VIEWS_H
