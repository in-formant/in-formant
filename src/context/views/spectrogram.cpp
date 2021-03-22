#include "spectrogram.h"
#include "../timings.h"
#include <iostream>
#include <qnamespace.h>

using namespace Main::View;

std::atomic_bool SpectrogramWorker::sQueued;

SpectrogramWorker::SpectrogramWorker()
{
    sQueued = false;
}

SpectrogramWorker::~SpectrogramWorker()
{
}

bool SpectrogramWorker::queued()
{
    return sQueued;
}

void SpectrogramWorker::renderImage(
        const rpm::vector<std::pair<double, SpectrogramCoefs>>& slices,
        double timeStart,
        double timeEnd,
        FrequencyScale frequencyScale,
        double minFrequency,
        double maxFrequency,
        double maxGain,
        int vw, int vh)
{
    sQueued = true;
    QImage image = QPainterWrapper::drawSpectrogram(
                        slices,
                        timeStart,
                        timeEnd,
                        frequencyScale,
                        minFrequency,
                        maxFrequency,
                        maxGain);
    emit imageRendered(image.scaled(vw, vh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), timeStart, timeEnd);
    sQueued = false;
}

Spectrogram::Spectrogram()
{
    qRegisterMetaType<rpm::vector<std::pair<double, SpectrogramCoefs>>>("rpm::vector<std::pair<double, SpectrogramCoefs>>");
    qRegisterMetaType<FrequencyScale>("FrequencyScale");

    auto* worker = new SpectrogramWorker;
    worker->moveToThread(&mImageRenderThread);
    QObject::connect(&mImageRenderThread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(this, &Spectrogram::renderImage, worker, &SpectrogramWorker::renderImage, Qt::QueuedConnection);
    QObject::connect(worker, &SpectrogramWorker::imageRendered, this, &Spectrogram::imageRendered, Qt::QueuedConnection);
    mImageRenderThread.start();
}

Spectrogram::~Spectrogram()
{
    mImageRenderThread.quit();
    mImageRenderThread.wait();
}

void Spectrogram::render(QPainterWrapper *painter, Config *config, DataStore *dataStore)
{
    dataStore->beginRead();

    auto& spectrogram = dataStore->getSpectrogram();
    auto& pitchTrack = dataStore->getPitchTrack();

    const QRect viewport = painter->viewport();

    const double viewDuration = config->getViewTimeSpan();
    const double timeDelay = 80.0 / 1000.0;
    const double timeEnd = dataStore->getTime() - timeDelay;
    const double timeStart = timeEnd - viewDuration;
    
    const double pitchTrackRenderInterval = 50.0 / 1000.0;
    const double formantTracksRenderInterval = 50.0 / 1000.0;

    rpm::vector<double> sound =
        !dataStore->getSoundTrack().empty() 
            ? dataStore->getSoundTrack().back()
            : rpm::vector<double>();

    painter->setTimeRange(timeStart, timeEnd);
  
    if (config->getViewShowSpectrogram()) {
        if (!SpectrogramWorker::queued()) {
            const double timeEndNoDelay = dataStore->getTime();
            const double timeStartNoDelay = timeEndNoDelay - viewDuration - 2 * timeDelay;

            rpm::vector<std::pair<double, SpectrogramCoefs>> slices(
                    spectrogram.lower_bound(timeStartNoDelay), spectrogram.upper_bound(timeEndNoDelay));

            emit renderImage(std::move(slices), timeStartNoDelay, timeEndNoDelay,
                    config->getViewFrequencyScale(),
                    config->getViewMinFrequency(),
                    config->getViewMaxFrequency(),
                    config->getViewMaxGain(),
                    viewport.width(),
                    viewport.height());
        }

        if (!mImage.isNull()) {
            const double x1 = painter->mapTimeToX(mImageTimeStart);
            const double x2 = painter->mapTimeToX(mImageTimeEnd);
            painter->drawPixmap(
                    QRectF(x1, 0, x2 - x1, painter->viewport().height()),
                    mImage,
                    QRectF({0,0}, mImage.size()));
        }
    }
    
    if (config->getViewShowPitch()) {
        const double lastPitchTrackTimeStart = mLastPitchTrackTimeStart;
        const double lastPitchTrackTimeEnd = mLastPitchTrackTimeEnd;

        if (mRenderedPitchTrack.isNull()
                || mRenderedPitchTrack.size() != viewport.size()
                || timeEnd - lastPitchTrackTimeEnd > pitchTrackRenderInterval
                || fabs((lastPitchTrackTimeEnd - lastPitchTrackTimeStart) - viewDuration) < 1e-8
                || mLastFrequencyScale != config->getViewFrequencyScale()
                || mLastMinFrequency != config->getViewMinFrequency()
                || mLastMaxFrequency != config->getViewMaxFrequency()) {
            
            painter->setPen(QPen(Qt::cyan, 10, Qt::SolidLine, Qt::RoundCap));
            auto image = painter->drawFrequencyTrack(pitchTrack.lower_bound(timeStart), pitchTrack.upper_bound(timeEnd), false);
            mRenderedPitchTrack.convertFromImage(image);
            mLastPitchTrackTimeStart = timeStart;
            mLastPitchTrackTimeEnd = timeEnd;
        }

        const double x1 = painter->mapTimeToX(mLastPitchTrackTimeStart);
        const double x2 = painter->mapTimeToX(mLastPitchTrackTimeEnd);
        painter->drawPixmap(
                QRectF(x1, 0, x2 - x1, painter->viewport().height()),
                mRenderedPitchTrack,
                QRectF({0,0}, mRenderedPitchTrack.size()));
    }

    if (config->getViewShowFormants()) {
        const int formantCount = config->getViewFormantCount();
        
        mRenderedFormantTracks.resize(formantCount);

        double lastFormantTracksTimeStart = mLastFormantTracksTimeStart;
        double lastFormantTracksTimeEnd = mLastFormantTracksTimeEnd;

        for (int i = 0; i < formantCount; ++i) {
            if (mRenderedFormantTracks[i].isNull()
                    || mRenderedFormantTracks[i].size() != viewport.size()
                    || timeEnd - lastFormantTracksTimeEnd > formantTracksRenderInterval
                    || fabs((lastFormantTracksTimeEnd - lastFormantTracksTimeStart) - viewDuration) < 1e-8
                    || mLastFrequencyScale != config->getViewFrequencyScale()
                    || mLastMinFrequency != config->getViewMinFrequency()
                    || mLastMaxFrequency != config->getViewMaxFrequency()) {
                
                auto& fi = dataStore->getFormantTrack(i);
                const auto [r, g, b] = config->getViewFormantColor(i);
                painter->setPen(QPen(QColor::fromRgbF(r, g, b), 8, Qt::SolidLine, Qt::RoundCap));
                auto image = painter->drawFrequencyTrack(fi.lower_bound(timeStart), fi.upper_bound(timeEnd), false);
                mRenderedFormantTracks[i].convertFromImage(image);
                mLastFormantTracksTimeStart = timeStart;
                mLastFormantTracksTimeEnd = timeEnd;
            }

            const double x1 = painter->mapTimeToX(mLastFormantTracksTimeStart);
            const double x2 = painter->mapTimeToX(mLastFormantTracksTimeEnd);
            painter->drawPixmap(
                    QRectF(x1, 0, x2 - x1, painter->viewport().height()),
                    mRenderedFormantTracks[i],
                    QRectF({0,0}, mRenderedFormantTracks[i].size()));
        }
    }

    painter->drawTimeAxis();
    
    if (mRenderedFrequencyScale.isNull()
            || mRenderedFrequencyScale.size() != viewport.size()
            || mLastFrequencyScale != config->getViewFrequencyScale()
            || mLastMinFrequency != config->getViewMinFrequency()
            || mLastMaxFrequency != config->getViewMaxFrequency()) {
        mRenderedFrequencyScale.convertFromImage(painter->drawFrequencyScale());
        mLastFrequencyScale = config->getViewFrequencyScale();
        mLastMinFrequency = config->getViewMinFrequency();
        mLastMaxFrequency = config->getViewMaxFrequency();
    }
    painter->drawPixmap(0, 0, mRenderedFrequencyScale);

    //painter->setTimeSeriesPen(QPen(QColor(0xFFA500), 2));
    //painter->drawTimeSeries(sound, 0, viewport.width(), -1, 1);

    dataStore->endRead();
}

void Spectrogram::imageRendered(QImage image, double timeStart, double timeEnd)
{
    mImage.convertFromImage(image);
    mImageTimeStart = timeStart;
    mImageTimeEnd = timeEnd;
}
