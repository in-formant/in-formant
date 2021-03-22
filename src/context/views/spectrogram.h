#ifndef MAIN_CONTEXT_VIEWS_SPECTROGRAM_H
#define MAIN_CONTEXT_VIEWS_SPECTROGRAM_H

#include "views.h"

namespace Main::View {

    class SpectrogramWorker : public QObject {
        Q_OBJECT

    public:
        SpectrogramWorker();
        virtual ~SpectrogramWorker();

        static bool queued();

    public slots:
        void renderImage(
                const rpm::vector<std::pair<double, SpectrogramCoefs>>& slices,
                double timeStart,
                double timeEnd,
                FrequencyScale frequencyScale,
                double minFrequency,
                double maxFrequency,
                double maxGain,
                int vw, int vh);

    signals:
        void imageRendered(QImage image, double timeStart, double timeEnd);

    private:
        static std::atomic_bool sQueued;
    };

    class Spectrogram : public QObject, public AbstractView {
        Q_OBJECT
   
    public:
        Spectrogram();
        virtual ~Spectrogram();

    protected:
        void render(QPainterWrapper *painter, Config *config, DataStore *dataStore) override;

    public slots:
        void imageRendered(QImage image, double timeStart, double timeEnd);

    signals:
        void renderImage(
                const rpm::vector<std::pair<double, SpectrogramCoefs>>& slices,
                double timeStart,
                double timeEnd,
                FrequencyScale frequencyScale,
                double minFrequency,
                double maxFrequency,
                double maxGain,
                int vw, int vh);

    private:
        QThread mImageRenderThread;
        QPixmap mImage;
        double mImageTimeStart;
        double mImageTimeEnd;

        QPixmap mRenderedFrequencyScale;
        FrequencyScale mLastFrequencyScale;
        double mLastMinFrequency;
        double mLastMaxFrequency;

        rpm::vector<QPixmap> mRenderedFormantTracks;
        double mLastFormantTracksTimeStart;
        double mLastFormantTracksTimeEnd;

        QPixmap mRenderedPitchTrack;
        double mLastPitchTrackTimeStart;
        double mLastPitchTrackTimeEnd;
    };

}

#endif
