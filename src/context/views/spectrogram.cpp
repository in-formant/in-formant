#include "views.h"

using namespace Main::View;

void Spectrogram::render(RendererBase *renderer, FontProvider *fp, Config *config, DataStore *dataStore)
{
    int fsz = config->getViewFontSize();

    int catchup = dataStore->beginRead();
   
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

    renderer->renderFrequencyScaleBar(fp->font(fsz - 3), fp->font(fsz - 4));
}
