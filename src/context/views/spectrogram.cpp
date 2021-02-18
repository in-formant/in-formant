#include "views.h"
#include <iostream>
#include <gaborator/gaborator.h>
#include <gaborator/render.h>
#include <qnamespace.h>

using namespace Main::View;

std::atomic_bool SpectrogramWorker::mQueued;

SpectrogramWorker::SpectrogramWorker()
{
    mQueued = false;
}

bool SpectrogramWorker::queued()
{
    return mQueued;
}

void SpectrogramWorker::render(
        double timeStart,
        double timeEnd,
        const rpm::vector<double>& amplitudes,
        int w, int h, int vw, int vh,
        QPainterWrapper::FrequencyScale scale,
        double minFrequency, double maxFrequency,
        double minGain, double maxGain) {
    mQueued = true;
    QImage image = QPainterWrapper::drawSpectrogram(
            amplitudes, w, h, vw, vh, scale,
            minFrequency, maxFrequency,
            minGain, maxGain);
    emit rendered(image, timeStart, timeEnd);
    mQueued = false;
}

Spectrogram::Spectrogram()
{
    qRegisterMetaType<rpm::vector<double>>("rpm::vector<double>");
    qRegisterMetaType<QPainterWrapper::FrequencyScale>("QPainterWrapper::FrequencyScale");
    
    auto *worker = new SpectrogramWorker;
    worker->moveToThread(&mRenderThread);
    QObject::connect(&mRenderThread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(this, &Spectrogram::renderSpectrogram, worker, &SpectrogramWorker::render);
    QObject::connect(worker, &SpectrogramWorker::rendered, this, &Spectrogram::spectrogramRendered);
    mRenderThread.start();
}

Spectrogram::~Spectrogram()
{
    mRenderThread.quit();
    mRenderThread.wait();
}

void Spectrogram::render(QPainterWrapper *painter, Config *config, DataStore *dataStore)
{
    dataStore->beginRead();

    auto& coefStore = dataStore->getSpectrogramCoefs();

    auto& pitchTrack = dataStore->getPitchTrack();

    const QRect viewport = painter->viewport();

    constexpr double viewDuration = 5.0;
    const double timeStart = dataStore->getTime() - viewDuration;
    const double timeEnd = dataStore->getTime();

    rpm::vector<double> sound =
        !dataStore->getSoundTrack().empty() 
            ? dataStore->getSoundTrack().back()
            : rpm::vector<double>();

    painter->setTimeRange(timeStart, timeEnd);
  
    if (config->getViewShowSpectrogram()) { 
        for (auto& coefs : coefStore) {
            double dfs = coefs.fs;

            int64_t t_end = dfs * timeEnd;
            int64_t t_dur = dfs * (timeEnd - timeStart);

            int x_scale_exp = 8;
            int y_scale_exp = -3;

            int64_t begin_band = coefs.analyzer.ff_bandpass_band((double) config->getViewMaxFrequency() / dfs);
            int64_t end_band = coefs.analyzer.ff_bandpass_band((double) config->getViewMinFrequency() / dfs);
            
            /*int64_t mid_band = coefs.analyzer.ff_bandpass_band(400.0 / dfs);*/

            int64_t x_origin = 0;
            int64_t y_origin = 0;
            int64_t x0 = (t_end - t_dur) >> x_scale_exp;
            int64_t x1 = t_end >> x_scale_exp;
            int64_t y0 = begin_band << -y_scale_exp;
            int64_t y1 = end_band << -y_scale_exp;
            int64_t i0 = t_end - t_dur;
            int64_t i1 = t_end;

            rpm::vector<double> amplitudes((x1 - x0) * (y1 - y0), 0.0);

            gaborator::render_incremental(
                    coefs.analyzer,
                    coefs.coefs,
                    gaborator::linear_transform(ldexp(1, x_scale_exp), x_origin),
                    gaborator::linear_transform(ldexp(1, y_scale_exp), y_origin),
                    x0, x1,
                    y0, y1,
                    i0, i1,
                    amplitudes.data(),
                    x1 - x0);
        
            if (!SpectrogramWorker::queued()) {
                emit renderSpectrogram(
                        timeStart,
                        timeEnd,
                        amplitudes,
                        x1 - x0, y1 - y0,
                        viewport.width(), viewport.height(),
                        config->getViewFrequencyScale(),
                        config->getViewMinFrequency(),
                        config->getViewMaxFrequency(),
                        config->getViewMinGain(),
                        config->getViewMaxGain());
            }
        }

        painter->drawImage(viewport.translated(painter->mapTimeToX(mTime), 0), mImage);
    }

    if (config->getViewShowPitch()) {
        painter->setPen(QPen(Qt::cyan, 6, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(pitchTrack.lower_bound(timeStart), pitchTrack.upper_bound(timeEnd));
    }

    if (config->getViewShowFormants()) {
        auto f1 = dataStore->getFormantTrack(0);
        auto f2 = dataStore->getFormantTrack(1);
        auto f3 = dataStore->getFormantTrack(2);

        painter->setPen(QPen(Qt::green, 7, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f1.lower_bound(timeStart), f1.upper_bound(timeEnd), false);

        painter->setPen(QPen(Qt::darkYellow, 7, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f2.lower_bound(timeStart), f2.upper_bound(timeEnd), false);
 
        painter->setPen(QPen(Qt::magenta, 7, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f3.lower_bound(timeStart), f3.upper_bound(timeEnd), false);
    }

    //painter->setTimeSeriesPen(QPen(QColor(0xFFA500), 2));
    //painter->drawTimeSeries(sound, 0, viewport.width(), -1, 1);

    dataStore->endRead();
}

void Spectrogram::spectrogramRendered(QImage image, double timeStart, double timeEnd)
{
    mImage = image;
    mTime = timeStart;
}
