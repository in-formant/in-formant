#include "views.h"
#include <iostream>
#include <gaborator/gaborator.h>
#include <gaborator/render.h>
#include <qnamespace.h>

using namespace Main::View;

void Spectrogram::render(QPainterWrapper *painter, Config *config, DataStore *dataStore)
{
    dataStore->beginRead();
    
    static QImage image;

    auto& coefStore = dataStore->getSpectrogramCoefs();

    auto& pitchTrack = dataStore->getPitchTrack();

    const QRect viewport = painter->viewport();

    constexpr double viewDuration = 5.0;
    const double timeStart = dataStore->getTime() - viewDuration;
    const double timeEnd = dataStore->getTime();

    rpm::vector<double> sound = (--dataStore->getSoundTrack().upper_bound(timeEnd))->second;

    painter->setTimeRange(timeStart, timeEnd);
  
    if (config->getViewShowSpectrogram()) { 
        for (auto& coefs : coefStore) {
            double dfs = coefs.fs;

            int64_t t_end = dfs * timeEnd;
            int64_t t_dur = dfs * (timeEnd - timeStart);

            int x_scale_exp = 9;
            int y_scale_exp = -1;

            int64_t begin_band = coefs.analyzer.ff_bandpass_band((double) config->getViewMaxFrequency() / dfs);
            int64_t end_band = coefs.analyzer.ff_bandpass_band((double) config->getViewMinFrequency() / dfs);

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

            image = painter->drawSpectrogramChunk(amplitudes, x1 - x0, y1 - y0);
        }

        painter->drawImage(viewport, image);
    }

    if (config->getViewShowPitch()) {
        painter->setPen(QPen(Qt::cyan, 5, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(pitchTrack.lower_bound(timeStart), pitchTrack.upper_bound(timeEnd));
    }

    if (config->getViewShowFormants()) {
        auto f1 = dataStore->getFormantTrack(0);
        auto f2 = dataStore->getFormantTrack(1);

        painter->setPen(QPen(Qt::green, 5, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f1.lower_bound(timeStart), f1.upper_bound(timeEnd));

        painter->setPen(QPen(Qt::darkYellow, 5, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f2.lower_bound(timeStart), f2.upper_bound(timeEnd));
    }

    painter->setTimeSeriesPen(QPen(QColor(0xFFA500), 2));
    painter->drawTimeSeries(sound, 0, viewport.width(), -1, 1);

    dataStore->endRead();
}

