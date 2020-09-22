#include "contextmanager.h"

using namespace Main;
using namespace std::chrono_literals;

void ContextManager::renderSpectrogram(RenderingContext &rctx)
{
    if (!isPaused) {
        Renderer::SpectrogramRenderData specRender;
        for (const auto& [frequency, intensity] : spectrogramTrack.back()) {
            specRender.push_back({frequency, intensity});
        }
        rctx.renderer->scrollSpectrogram(specRender, spectrogramCount);
    }
    rctx.renderer->renderSpectrogram();

    Renderer::FrequencyTrackRenderData pitchTrackRender;
    for (const auto& x : pitchTrack) {
        if (x > 0)
            pitchTrackRender.push_back(std::make_optional<float>(x));
        else
            pitchTrackRender.emplace_back(std::nullopt);
    }
    rctx.renderer->renderFrequencyTrack(pitchTrackRender, 4.0f, 0.0f, 1.0f, 1.0f);

    std::vector<Renderer::FormantTrackRenderData> formantTrackRender(numFormantsToRender);
    for (const auto& formants : formantTrack) {
        int i = 0;
        for (const auto& formant : formants) {
            formantTrackRender[i].push_back(std::make_optional<Analysis::FormantData>(formant));
            if (++i >= numFormantsToRender) break;
        }
        for (int j = i; j < numFormantsToRender; ++j)
            formantTrackRender[j].emplace_back(std::nullopt);
    }
    for (int i = 0; i < numFormantsToRender; ++i) {
        const auto [r, g, b] = formantColors[i];
        rctx.renderer->renderFormantTrack(formantTrackRender[i], r, g, b);
    }
 
    auto& tickLabelFont = primaryFont->with(uiFontSize - 4, rctx.target.get());
    rctx.renderer->renderFrequencyScaleBar(tickLabelFont, tickLabelFont);

    if (durLoop > 0us) {
        auto& font = primaryFont->with(uiFontSize - 3, rctx.target.get());
        
        int em = std::get<3>(font.queryTextSize("M"));

        float y;
        float tx, ty, tw, th;

        std::stringstream ss;

        y = 15;

        ss.str("");
        ss << "Loop cycle took " << (durLoop.count() / 1000.0f) << " ms";
        std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
        rctx.renderer->renderText(
                font,
                ss.str(),
                15,
                y,
                0.7f, 0.7f, 0.7f);
        y += em + 10;

        ss.str("");
        ss << "F1: Reopen oscilloscope"; 
        std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
        rctx.renderer->renderText(
                font,
                ss.str(),
                15,   
                y,
                1.0f, 1.0f, 1.0f);
        y += em + 10;

        ss.str("");
        ss << "F2: Reopen FFT spectrum";
        std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
        rctx.renderer->renderText(
                font,
                ss.str(),
                15,
                y,
                1.0f, 1.0f, 1.0f);  
        y += em + 10;

        ss.str("");
        ss << "P: " << (isPaused ? "Resume" : "Pause");
        std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
        rctx.renderer->renderText(
                font,
                ss.str(),
                15,
                y,
                1.0f, 1.0f, 1.0f);

    }
}

void ContextManager::eventSpectrogram(RenderingContext &rctx)
{ 
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
#endif
}

