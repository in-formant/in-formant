#include "views.h"
#include "../timings.h"
#include <iostream>
#include <qnamespace.h>

using namespace Main::View;

Spectrogram::Spectrogram()
{
    qRegisterMetaType<rpm::vector<double>>("rpm::vector<double>");
    qRegisterMetaType<FrequencyScale>("FrequencyScale");
}

Spectrogram::~Spectrogram()
{
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
        painter->setPen(QPen(Qt::cyan, 8, Qt::SolidLine, Qt::RoundCap));
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

