#if ! ( defined(__EMSCRIPTEN__) || defined(ANDROID) || defined(__ANDROID__) )

#include "contextmanager.h"
#include <iostream>

using namespace Main;

void ContextManager::initSynthUI()
{
    mSynthFields = {
        {
            .labelText = "Master gain: ",
            .min = 0.0, .max = 1.0,
            .value = [this]() { return synth.getMasterGain(); },
            .update = [this](float v) { synth.setMasterGain(v); }, 
            .barText = [this]() { return std::to_string((int) std::round(100 * synth.getMasterGain())) + " %"; },
        },
        {
            .labelText = "Noise source gain: ",
            .min = 0.0, .max = 1.0,
            .value = [this]() { return synth.getNoiseGain(); },
            .update = [this](float v) { synth.setNoiseGain(v); },
            .barText = [this]() { return std::to_string((int) std::round(100 * synth.getNoiseGain())) + " %"; },
        },
        { 
            .labelText = "Glottal source gain: ",
            .min = 0.0, .max = 1.0,
            .value = [this]() { return synth.getGlotGain(); },
            .update = [this](float v) { synth.setGlotGain(v); },
            .barText = [this]() { return std::to_string((int) std::round(100 * synth.getGlotGain())) + " %"; },
        },
        {
            .labelText = "Glottal pitch: ",
            .min = 60, .max = 600,
            .value = [this]() { return synth.getGlotPitch(); },
            .update = [this](float v) { synth.setGlotPitch(v); },
            .barText = [this]() { return std::to_string((int) std::round(synth.getGlotPitch())) + " Hz"; },
        },
        {
            .labelText = "Glottal source Rd: ",
            .min = 0.7, .max = 2.6,
            .value = [this]() { return synth.getGlotRd(); },
            .update = [this](float v) { synth.setGlotRd(v); },
            .barText = [this]() {
                                    std::ostringstream out;
                                    out.precision(1);
                                    out << std::fixed
                                        << (std::round(synth.getGlotRd() * 10) / 10);
                                    return out.str();
                                },
        },
        {
            .labelText = "Glottal source Tc: ",
            .min = 0.9, .max = 1.0,
            .value = [this]() { return synth.getGlotTc(); },
            .update = [this](float v) { synth.setGlotTc(v); },
            .barText = [this]() {
                                    std::ostringstream out;
                                    out.precision(2);
                                    out << std::fixed
                                        << (std::round(synth.getGlotTc() * 100) / 100);
                                    return out.str();
                                },
        },
        {
            .labelText = "Vocal tract filter shift: ",
            .min = 0.5, .max = 2.0,
            .value = [this]() { return synth.getFilterShift(); },
            .update = [this](float v) { synth.setFilterShift(v); },
            .barText = [this]() { return std::to_string((int) std::round(100 * synth.getFilterShift())) + " %"; },
        },
        {
            .labelText = "Voicing: ",
            .min = 0.0, .max = 1.0,
            .value = [this]() { return (float) ((int) synth.isVoiced()); },
            .update = [this](float v) { synth.setVoiced((bool) ((int) std::round(v))); },
            .barText = [this]() { return synth.isVoiced() ? "On" : "Off"; },
        },
    };
}

void ContextManager::renderSynth(RenderingContext &rctx)
{
    auto& font = FONT(primaryFont, uiFontSize - 4, rctx);
  
    int em = std::get<3>(font.queryTextSize("M"));

    int x = 10, x2;
    int y = 10;
    int padding = 5;
    int margin = 25;

    int sliderWidth = 300;
    int sliderKnobWidth = 16;

    for (auto& f : mSynthFields) { 
        const auto [tx, ty, tw, th] = font.queryTextSize(f.labelText);
        rctx.renderer->renderText(font, f.labelText, x, y + padding, 1.0f, 1.0f, 1.0f);
       
        const float val = f.value();
        const auto valstr = f.barText();

        const float frac = (val - f.min) / (f.max - f.min);

        x2 = x + tw + margin;

        if (f.labelText == "Voicing: ") {
            sliderWidth = 100;
        }
        else {
            sliderWidth = 300;
        }
        
        std::tie(f.x, f.y, f.w, f.h) = rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth, false);
        rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth * frac, true);
        rctx.renderer->renderRoundedRect(x2 + sliderWidth * frac - sliderKnobWidth / 2, y - 2, sliderKnobWidth, em + 14, 0.5f, 0.5f, 0.5f, 1.0f);

        int vsw = std::get<2>(font.queryTextSize(valstr));
        rctx.renderer->renderText(font, valstr, x2 + padding + sliderWidth / 2 - vsw / 2, y + padding, 1.0f, 1.0f, 1.0f); 
    
        y += em + 2 * padding + margin;
    }
}

void ContextManager::eventSynth(RenderingContext &rctx)
{
    int iframe = 
        useFrameCursor ? std::min(std::max<int>(std::round(specMX * spectrogramCount), 0), spectrogramCount - 1)
                       : spectrogramCount - 1;

    synth.setFormants(formantTrack[iframe]);
    
    const auto [mx, my] = rctx.target->getMousePosition();

    int tw, th;
    int rw, rh;
    rctx.target->getSize(&tw, &th);
    rctx.target->getSizeForRenderer(&rw, &rh);
   
    bool settingsChanged = false;

    if (rctx.target->isMousePressed(SDL_BUTTON_LEFT)) {
        for (auto& f : mSynthFields) {
            int x = (f.x * tw) / rw;
            int w = (f.w * tw) / rw;
            int y = (f.y * th) / rh;
            int h = (f.h * th) / rh;

            if (mx >= x && mx < x + w
                    && my >= y && my < y + h) {
                float oldValue = f.value();
                f.update(f.min + (f.max - f.min) * (mx - x) / (float) w);
                if (oldValue != f.value()) {
                    settingsChanged = true;
                }
            }
        }
    }
}

#endif
