#include "contextmanager.h"

using namespace Main;
using namespace std::chrono_literals;

void ContextManager::renderSpectrogram(RenderingContext &rctx)
{
    if (!isPaused) {
        Renderer::SpectrogramRenderData specRender;
        
        if (displayLpSpec) { 
            auto lpSpec = nodeIOs["linpred"][1]->as<Nodes::IO::AudioSpec>();
            for (int i = 0; i < lpSpec->getLength(); ++i) {
                float frequency = (i * lpSpec->getSampleRate() / 2.0f) / lpSpec->getLength();
                specRender.push_back({frequency, lpSpec->getConstData()[i]});
            }
        }
        else {
            for (const auto& [frequency, intensity] : spectrogramTrack.back()) {
                specRender.push_back({frequency, intensity});
            }
        }

        rctx.renderer->scrollSpectrogram(specRender, spectrogramCount);
    }
    rctx.renderer->renderSpectrogram();

    std::vector<Renderer::FormantTrackRenderData> formantTrackRender(numFormantsToRender);
    int n = 0;
    for (const auto& formants : formantTrack) {
        int i = 0;
        if (pitchTrack[n] > 0) {
            for (const auto& formant : formants) {
                formantTrackRender[i].push_back(std::make_optional<Analysis::FormantData>(formant));
                if (++i >= numFormantsToRender) break;
            }
        }
        for (int j = i; j < numFormantsToRender; ++j)
            formantTrackRender[j].emplace_back(std::nullopt);
        n++;
    }

    for (int i = 0; i < numFormantsToRender; ++i) {
        const auto [r, g, b] = formantColors[i];
        rctx.renderer->renderFormantTrack(formantTrackRender[i], r, g, b);
    }

    Renderer::FrequencyTrackRenderData pitchTrackRender;
    for (const auto& x : pitchTrack) {
        if (x > 0)
            pitchTrackRender.push_back(std::make_optional<float>(x));
        else
            pitchTrackRender.emplace_back(std::nullopt);
    }
    rctx.renderer->renderFrequencyTrack(pitchTrackRender, 6.0f, 0.0f, 1.0f, 1.0f);
 
    auto& tickLabelFont = primaryFont->with(uiFontSize - 4, rctx.target.get());
    rctx.renderer->renderFrequencyScaleBar(tickLabelFont, tickLabelFont);

    if (durLoop > 0us) {
        auto& font = primaryFont->with(uiFontSize - 3, rctx.target.get());
        auto& smallerFont = primaryFont->with(uiFontSize - 5, rctx.target.get());
        
        int em = std::get<3>(font.queryTextSize("M"));
        int smallerEm = std::get<3>(smallerFont.queryTextSize("M"));

        float y;
        float tx, ty, tw, th;

        std::stringstream ss;

        y = 15;

#if ! ( defined(ANDROID) || defined(__ANDROID__) ) 
        const std::vector<std::string> keyLegends = {
            "F1: Open oscilloscope",
            "F2: Open FFT spectrum",
            "P: Pause/resume analysis",
            "N: Play filtered noise",
            "L: Toggle spectrogram/LP spectra",
            "F: Toggle frame cursor",
        };
#else
        const std::array<std::string, 0> keyLegends {};
#endif

        for (const auto& str : keyLegends) {
            rctx.renderer->renderText(
                    font,
                    str,
                    15,   
                    y,
                    1.0f, 1.0f, 1.0f);
            y += em + 10;
        }

        std::vector<std::string> bottomStrings;
        float frequency;

        frequency = rctx.renderer->renderFrequencyCursor(specMX, specMY);
        
        ss.str("");
        ss << "Cursor: " << std::round(frequency) << " Hz";
        bottomStrings.push_back(ss.str());

        int iframe = 
            useFrameCursor ? rctx.renderer->renderFrameCursor(specMX, specMY, spectrogramCount)
                           : spectrogramCount - 1;

        frequency = pitchTrack[iframe];

        ss.str("");
        ss << "Pitch: ";
        if (frequency > 0)
            ss << (10 * std::round(frequency)) << " Hz";
        else
            ss << "unvoiced";
        bottomStrings.push_back(ss.str());

        auto& formants = formantTrack[iframe];
        int i = 1;
        for (auto it = formants.begin(); it != formants.end(); ++it) {
            frequency = it->frequency;

            ss.str("");
            ss << "R" << i << ": " << std::round(frequency) << " Hz";
            bottomStrings.push_back(ss.str());
            
            i++;
        }

        frequency = pitchTrack.back(); 

        y += smallerEm;

        std::tie(tx, ty, tw, th) = smallerFont.queryTextSize("Cursor: 55555 Hz");

        int maxWidth = tw;
        static int maxHeight = 0;
        th = bottomStrings.size() * (smallerEm + 10) - 10;
        if (th > maxHeight) {
            maxHeight = th;
        }

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

    const auto [mx, my] = rctx.target->getMousePosition();
    int mw, mh;
    rctx.target->getSize(&mw, &mh);

    specMX = (float) mx / (float) mw;
    specMY = (float) my / (float) mh;

    if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_L)) {
        displayLpSpec = !displayLpSpec;
    }

    if (rctx.target->isKeyPressedOnce(SDL_SCANCODE_F)) {
        useFrameCursor = !useFrameCursor;
    }
}

