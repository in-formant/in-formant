#include "views.h"
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
    QObject::connect(this, &Spectrogram::renderImage, worker, &SpectrogramWorker::renderImage);
    QObject::connect(worker, &SpectrogramWorker::imageRendered, this, &Spectrogram::imageRendered);
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
    auto& f1 = dataStore->getFormantTrack(0);
    auto& f2 = dataStore->getFormantTrack(1);
    auto& f3 = dataStore->getFormantTrack(2);

    const QRect viewport = painter->viewport();

    const double viewDuration = config->getViewTimeSpan();
    const double timeStart = dataStore->getTime() - viewDuration;
    const double timeEnd = dataStore->getTime();

    rpm::vector<double> sound =
        !dataStore->getSoundTrack().empty() 
            ? dataStore->getSoundTrack().back()
            : rpm::vector<double>();

    painter->setTimeRange(timeStart, timeEnd);
  
    if (config->getViewShowSpectrogram()) {
        if (!SpectrogramWorker::queued()) {
            rpm::vector<std::pair<double, SpectrogramCoefs>> slices(
                    spectrogram.lower_bound(timeStart), spectrogram.upper_bound(timeEnd));
            emit renderImage(std::move(slices), timeStart, timeEnd,
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
            painter->drawImage(
                    QRectF(x1, 0, x2 - x1, painter->viewport().height()),
                    mImage);
        }
    }
    
    if (config->getViewShowPitch()) {
        painter->setPen(QPen(Qt::cyan, 10, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(pitchTrack.lower_bound(timeStart), pitchTrack.upper_bound(timeEnd), false);
    }

    if (config->getViewShowFormants()) {
        double r, g, b;

        std::tie(r, g, b) = config->getViewFormantColor(0);
        painter->setPen(QPen(QColor::fromRgbF(r, g, b), 8, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f1.lower_bound(timeStart), f1.upper_bound(timeEnd), false);

        std::tie(r, g, b) = config->getViewFormantColor(1);
        painter->setPen(QPen(QColor::fromRgbF(r, g, b), 8, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f2.lower_bound(timeStart), f2.upper_bound(timeEnd), false);
 
        std::tie(r, g, b) = config->getViewFormantColor(2);
        painter->setPen(QPen(QColor::fromRgbF(r, g, b), 8, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f3.lower_bound(timeStart), f3.upper_bound(timeEnd), false);
    }

    painter->drawTimeAxis();
    painter->drawFrequencyScale();

    //painter->setTimeSeriesPen(QPen(QColor(0xFFA500), 2));
    //painter->drawTimeSeries(sound, 0, viewport.width(), -1, 1);

    dataStore->endRead();
}

void Spectrogram::imageRendered(QImage image, double timeStart, double timeEnd)
{
    mImage = image;
    mImageTimeStart = timeStart;
    mImageTimeEnd = timeEnd;
}
