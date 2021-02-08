#include "views.h"
#include <iostream>
#include <gaborator/gaborator.h>
#include <gaborator/render.h>
#include <qnamespace.h>

using namespace Main::View;

void Spectrogram::render(QPainterWrapper *painter, Config *config, DataStore *dataStore)
{
    int fsz = config->getViewFontSize();

    /*int catchup = dataStore->beginRead();
   
    int trackLength = dataStore->getTrackLength(); 
    int viewFormantCount = config->getViewFormantCount();

    if (catchup > 0) {
        int n;

        auto spectrumTail = dataStore->getSoundSpectrumTrack().tail(catchup);

        for (const auto& spectrum : spectrumTail) {
            mSpectrumRender.clear();
            for (const auto& [frequency, intensity] : spectrum) {
                mSpectrumRender.push_back({frequency, intensity});
            }
            renderer->scrollSpectrogram(mSpectrumRender, trackLength);
        }

        mFormantTracksRender.resize(viewFormantCount);
        for (int i = 0; i < viewFormantCount; ++i) {
            mFormantTracksRender[i].resize(trackLength);
        }

        n = 0;
        for (auto& formants : dataStore->getFormantTrack()) {
            int i = 0;
            for (auto& formant : formants) {
                mFormantTracksRender[i][n] = formant;
                if (++i >= viewFormantCount) break;
            }
            for (int j = i; j < viewFormantCount; ++j)
                mFormantTracksRender[j][n] = std::nullopt;
            ++n;
        }

        mPitchTrackRender.resize(trackLength);
        n = 0;
        for (auto& x : dataStore->getPitchTrack()) {
            mPitchTrackRender[n++] = (x > 0) ? std::make_optional(x) : std::nullopt;
        }
    }

    dataStore->endRead();

    if (config->getViewShowSpectrogram()) {
        renderer->renderSpectrogram();
    }

    if (config->getViewShowPitch()) {
        renderer->renderFrequencyTrack(mPitchTrackRender, 6.0f, 0.0f, 1.0f, 1.0f);
    }

    if (config->getViewShowFormants()) {
        for (int i = 0; i < viewFormantCount; ++i) {
            const auto [r, g, b] = config->getViewFormantColor(i);
            renderer->renderFormantTrack(mFormantTracksRender[i], 4.0f, r, g, b);
        }
    }

    renderer->renderFrequencyScaleBar(fp->font(fsz - 3), fp->font(fsz - 4));*/

    dataStore->beginRead();

    rpm::vector<double> sound = dataStore->getSoundTrack().back();
    
    static QImage image;

    auto& analyzerOpt = dataStore->getSpectrogramAnalyzer();
    auto& coefsOpt = dataStore->getSpectrogramCoefs();

    auto& pitchTrack = dataStore->getPitchTrack();

    const QRect viewport = painter->viewport();

    constexpr double viewDuration = 5.0;
    const double timeStart = dataStore->getTime() - viewDuration;
    const double timeEnd = dataStore->getTime();

    painter->setTimeRange(timeStart, timeEnd);
    
    if (coefsOpt.has_value()) {
        auto& analyzer = *analyzerOpt;
        auto& coefs = *coefsOpt;
        int64_t t_end = 48000 * timeEnd;
        int64_t t_dur = 48000 * (timeEnd - timeStart);

        int x_scale_exp = 8;
        int y_scale_exp = -1;

        int64_t x_origin = 0;
        int64_t y_origin = analyzer.bandpass_bands_begin();
        int64_t x0 = (t_end - t_dur) >> x_scale_exp;
        int64_t x1 = t_end >> x_scale_exp;
        int64_t y0 = 0;
        int64_t y1 = (analyzer.bandpass_bands_end() - analyzer.bandpass_bands_begin()) << -y_scale_exp;
        int64_t i0 = t_end - t_dur;
        int64_t i1 = t_end;
    
        rpm::vector<double> amplitudes((x1 - x0) * (y1 - y0), 0.0);

        gaborator::render_incremental(
                analyzer,
                coefs,
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

