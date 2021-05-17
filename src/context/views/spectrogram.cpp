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
    const double realTimeEnd = dataStore->getRealTime();

    dataStore->beginWrite();
    constexpr double keepDuration = 50.0;
    const double keepTimeStart = realTimeEnd - keepDuration;
    dataStore->getSpectrogram().remove_before(keepTimeStart);
    dataStore->getSoundTrack().remove_before(keepTimeStart);
    dataStore->getGifTrack().remove_before(keepTimeStart);
    dataStore->endWrite();

    dataStore->beginRead();

    auto& spectrogram = dataStore->getSpectrogram();
    auto& pitchTrack = dataStore->getPitchTrack();

    const double viewDuration = config->getViewTimeSpan();
    const double timeDelay = 80.0 / 1000.0;
    const double timeEnd = realTimeEnd - timeDelay;
    const double timeStart = timeEnd - viewDuration;

    rpm::vector<double> sound =
        !dataStore->getSoundTrack().empty() 
            ? dataStore->getSoundTrack().back()
            : rpm::vector<double>();

    painter->setTimeRange(timeStart, timeEnd);
  
    if (config->getViewShowSpectrogram()) {
        rpm::vector<std::pair<double, SpectrogramCoefs>> slices(
                spectrogram.lower_bound(timeStart), spectrogram.upper_bound(timeEnd));
        std::sort(slices.begin(), slices.end(), [](auto& a, auto& b) { return a.first > b.first; });
        painter->drawSpectrogram(slices);
    }
    
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
