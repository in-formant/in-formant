#include "contextmanager.h"

using namespace Main;
using namespace std::chrono_literals;

void ContextManager::renderSpectrogram(RenderingContext &rctx)
{
    Renderer::SpectrogramRenderData specRender;
    for (const auto& [frequency, intensity] : spectrogramTrack.back()) {
        specRender.push_back({frequency, intensity});
    }
    rctx.renderer->renderSpectrogram(specRender, spectrogramCount);

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
        auto& font = primaryFont->with(uiFontSize, rctx.target.get());

        const auto [tx, ty, tw, th] = font.queryTextSize("M");

        std::stringstream ss;
        ss << "Loop cycle took " << (durLoop.count() / 1000.0f) << " ms";
        ss.flush();
        rctx.renderer->renderText(
                font,
                ss.str(),
                20,
                20,
                1.0f, 0.5f, 1.0f);

        float processingFrac = (float) durProcessing.count() / (float) durLoop.count();
        float renderingFrac = (float) durRendering.count() / (float) durLoop.count();

        ss.str("");
        ss << "- Processing: " << std::round(100 * processingFrac) << "%";
        ss.flush();
        rctx.renderer->renderText(
                font,
                ss.str(),
                20,
                20 + th + 10,
                1.0f, 0.5f, 1.0f);

        ss.str("");
        ss << "- Rendering: " << std::round(100 * renderingFrac) << "%";
        ss.flush();
        rctx.renderer->renderText(
                font,
                ss.str(),
                20,
                20 + 2 * (th + 10),
                1.0f, 0.5f, 1.0f);
    }
}

void ContextManager::eventSpectrogram(RenderingContext &rctx)
{ 
#ifndef __EMSCRIPTEN__
    if (rctx.target->isKeyPressed(SDL_SCANCODE_F1)) {
        ctx->renderingContexts["Oscilloscope"].target->show();
    }
    else if (rctx.target->isKeyPressed(SDL_SCANCODE_F2)) {
        ctx->renderingContexts["FFT Spectrum"].target->show();
    }
#endif
}

