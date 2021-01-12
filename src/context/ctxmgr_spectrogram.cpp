#include "contextmanager.h"
#include <iomanip>

using namespace Main;
using namespace std::chrono_literals;

void ContextManager::scrollSpectrogram(RenderingContext &rctx)
{
    Renderer::SpectrogramRenderData specRender;
   
    auto& slice = displayLpSpec ? lpSpecTrack.back() : spectrogramTrack.back();

    for (const auto& [frequency, intensity] : slice) {
        specRender.push_back({frequency, intensity});
    }

    rctx.renderer->scrollSpectrogram(specRender, spectrogramCount);
}

void ContextManager::renderSpectrogram(RenderingContext &rctx)
{
    if (!isPaused) {
        scrollSpectrogram(rctx);
    }

    rctx.renderer->renderSpectrogram();

    if (displayFormantTracks) {
        std::vector<Renderer::FormantTrackRenderData> formantTrackRender(numFormantsToRender);
        int n = 0;
        for (auto& formants : formantTrack) {
            int i = 0;
            if (pitchTrack[n] > 0) {
                for (auto formant : formants) {
                    formantTrackRender[i].emplace_back(formant);
                    if (++i >= numFormantsToRender) break;
                }
            }
            for (int j = i; j < numFormantsToRender; ++j)
                formantTrackRender[j].emplace_back(std::nullopt);
            n++;
        }

        for (int i = 0; i < numFormantsToRender; ++i) {
            const auto [r, g, b] = formantColors[i];
            rctx.renderer->renderFormantTrack(formantTrackRender[i], 4.0f, r, g, b);
        }
    }

    if (displayPitchTracks) {
        Renderer::FrequencyTrackRenderData pitchTrackRender;
        for (auto x : pitchTrack) {
            if (x > 0)
                pitchTrackRender.emplace_back(x); 
            else
                pitchTrackRender.emplace_back(std::nullopt);
        }
        rctx.renderer->renderFrequencyTrack(pitchTrackRender, 6.0f, 0.0f, 1.0f, 1.0f);
    }
 
    auto& tickLabelFont = FONT(primaryFont, uiFontSize - 5, rctx);
    rctx.renderer->renderFrequencyScaleBar(tickLabelFont, tickLabelFont);

    if (durLoop > 0us) {
        auto& font = FONT(primaryFont, uiFontSize - 4, rctx);
        auto& smallerFont = FONT(primaryFont, uiFontSize - 5, rctx);
        
        int em = std::get<3>(font.queryTextSize("M"));
        int smallerEm = std::get<3>(smallerFont.queryTextSize("M"));

        float y;
        float tx, ty, tw, th;

        std::stringstream ss;

        y = 15;

        std::vector<std::string> bottomStrings;
        float frequency;

        frequency = rctx.renderer->renderFrequencyCursor(specMX, specMY);
        
        ss.str("");
        ss << "Cursor: " << std::round(frequency) << " Hz";
        bottomStrings.push_back(ss.str());

        int iframe = 
            useFrameCursor ? rctx.renderer->renderFrameCursor(specMX, specMY, spectrogramCount)
                           : spectrogramCount - 1;

        if (displayPitchTracks) {
            frequency = pitchTrack[iframe];

            ss.str("");
            ss << "Pitch: ";
            if (frequency > 0)
                ss << ((10.0f * std::round(frequency)) / 10.0f) << " Hz";
            else
                ss << "unvoiced";
            bottomStrings.push_back(ss.str());
        }

        if (displayFormantTracks) {
            static std::vector<float> lastFormants;

            auto& formants = formantTrack[iframe];
            int i = 1;
            for (const auto& formant : formants) {
                if (i > lastFormants.size()) {
                    lastFormants.resize(i, 0.0f);
                }

                frequency = lastFormants[i - 1] = 0.8f * lastFormants[i - 1] + 0.2f * formant.frequency;

                ss.str("");
                ss << "R" << i << ": "
                   << std::fixed << std::setprecision(2)
                   << (std::round(frequency / 5) / 200) << " kHz";
                bottomStrings.push_back(ss.str());
                
                i++;
            }
        }

        frequency = pitchTrack.back(); 

        y += smallerEm;

        std::tie(tx, ty, tw, th) = smallerFont.queryTextSize("Cursor: 55555 Hz");

        int maxWidth = tw;
        static float maxHeight = 0;
        th = bottomStrings.size() * (smallerEm + 10) - 10;
        maxHeight = 0.1 * th + (1 - 0.1) * maxHeight;

        for (const auto& str : bottomStrings) {
            const auto [tx, ty, tw, th] = smallerFont.queryTextSize(str);
            if (tw > maxWidth) {
                maxWidth = tw;
            }
        }

        rctx.renderer->renderRoundedRect(
                15, y, maxWidth + 16, maxHeight + 16,
                0.157f, 0.165f, 0.212f, 0.8f);

        for (const auto& str : bottomStrings) {
            rctx.renderer->renderText(
                    smallerFont,
                    str,
                    15 + 8,
                    y + 8,
                    0.973f, 0.973f, 0.949f);
            y += smallerEm + 10;
        }

        if (displayLegends) {
            int h;
            rctx.target->getSize(nullptr, &h);

            ss.str("");
            ss << "Loop cycle took " << (durLoop.count() / 1000.0f) << " ms";
            rctx.renderer->
            renderText(
                    smallerFont,
                    ss.str(),
                    15,
                    h - 15 - smallerEm,
                    1.0f, 1.0f, 1.0f);
        }
    }
}

void ContextManager::eventSpectrogram(RenderingContext &rctx)
{
/* 
#ifndef __EMSCRIPTEN__
    if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_F1)) {
        auto& target = ctx->renderingContexts["Oscilloscope"].target;
        if (!target->isVisible()) {
            target->show();
        }
    }
    if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_F2)) {
        auto& target = ctx->renderingContexts["FFT spectrum"].target;
        if (!target->isVisible()) {
            target->show();
        }
    }
    if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_F3)) {
        auto& target = ctx->renderingContexts["Synthesizer"].target;
        if (!target->isVisible()) {
            target->show();
        }
    }
#endif
*/
    const auto [mx, my] = rctx.target->getMousePosition();
    int mw, mh;
    rctx.target->getSize(&mw, &mh);

    if (rctx.target->isMousePressed(0)) {
        specMX = (float) mx / (float) mw;
        specMY = (float) my / (float) mh;
    }
}

