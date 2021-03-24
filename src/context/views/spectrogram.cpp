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
    /*QImage image = QPainterWrapper::drawSpectrogram(
                        slices,
                        timeStart,
                        timeEnd,
                        frequencyScale,
                        minFrequency,
                        maxFrequency,
                        maxGain);
    emit imageRendered(image.scaled(vw, vh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), timeStart, timeEnd);*/
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
        rpm::vector<std::pair<double, SpectrogramCoefs>> slices(
                spectrogram.lower_bound(timeStart), spectrogram.upper_bound(timeEnd));
        painter->drawSpectrogram(slices);
    }


        /*if (!SpectrogramWorker::queued()) {
            const double timeEndNoDelay = dataStore->getTime();
            const double timeStartNoDelay = timeEndNoDelay - viewDuration - 2 * timeDelay;


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
    }*/
    
    if (config->getViewShowPitch()) {
        painter->drawFrequencyTrack(
                pitchTrack.lower_bound(timeStart),
                pitchTrack.upper_bound(timeEnd),
                3, Qt::cyan);
    }

    if (config->getViewShowFormants()) {
        const int formantCount = config->getViewFormantCount();

        for (int i = 0; i < formantCount; ++i) {
            auto& fi = dataStore->getFormantTrack(i);
            const auto [r, g, b] = config->getViewFormantColor(i);
        
            painter->drawFrequencyTrack(
                    fi.lower_bound(timeStart),
                    fi.upper_bound(timeEnd),
                    3, QColor::fromRgbF(r, g, b));
        }
    }

    painter->drawTimeAxis();
    painter->drawFrequencyScale();

    dataStore->endRead();
}

void Spectrogram::imageRendered(QImage image, double timeStart, double timeEnd)
{
    mImage.convertFromImage(image);
    mImageTimeStart = timeStart;
    mImageTimeEnd = timeEnd;
}
