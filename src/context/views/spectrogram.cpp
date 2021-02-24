#include "views.h"
#include "../timings.h"
#include <iostream>
#include <gaborator/gaborator.h>
#include <gaborator/render.h>
#include <qnamespace.h>

using namespace Main::View;

Spectrogram::Spectrogram()
    : mThreadPool(this)
{
    mThreadPool.setMaxThreadCount(numBlocksMax);

    qRegisterMetaType<rpm::vector<double>>("rpm::vector<double>");
    qRegisterMetaType<QPainterWrapper::FrequencyScale>("QPainterWrapper::FrequencyScale");

    std::fill(mRendering.begin(), mRendering.end(), false);
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

    constexpr double viewDuration = 5.0;
    const double timeStart = dataStore->getTime() - viewDuration;
    const double timeEnd = dataStore->getTime();

    rpm::vector<double> sound =
        !dataStore->getSoundTrack().empty() 
            ? dataStore->getSoundTrack().back()
            : rpm::vector<double>();

    painter->setTimeRange(timeStart, timeEnd);
  
    if (config->getViewShowSpectrogram()) {
        painter->drawSpectrogram(spectrogram.lower_bound(timeStart), spectrogram.upper_bound(timeEnd));
    }
    
    if (config->getViewShowPitch()) {
        painter->setPen(QPen(Qt::cyan, 6, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(pitchTrack.lower_bound(timeStart), pitchTrack.upper_bound(timeEnd));
    }

    if (config->getViewShowFormants()) {
        painter->setPen(QPen(Qt::green, 7, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f1.lower_bound(timeStart), f1.upper_bound(timeEnd), false);

        painter->setPen(QPen(Qt::darkYellow, 7, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f2.lower_bound(timeStart), f2.upper_bound(timeEnd), false);
 
        painter->setPen(QPen(Qt::magenta, 7, Qt::SolidLine, Qt::RoundCap));
        painter->drawFrequencyTrack(f3.lower_bound(timeStart), f3.upper_bound(timeEnd), false);
    }

    painter->drawFrequencyScale();

    //painter->setTimeSeriesPen(QPen(QColor(0xFFA500), 2));
    //painter->drawTimeSeries(sound, 0, viewport.width(), -1, 1);

    dataStore->endRead();
}

