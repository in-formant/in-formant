#if ! ( defined(__EMSCRIPTEN__) || defined(ANDROID) || defined(__ANDROID__) )

#include "contextmanager.h"
#include <iostream>

using namespace Main;

static std::array<int, 4> posMasterGain;
static std::array<int, 4> posNoiseGain;
static std::array<int, 4> posGlotGain;
static std::array<int, 4> posGlotPitch;
static std::array<int, 4> posGlotRd;
static std::array<int, 4> posFilterShift;

constexpr float glotPitchMin = 70;
constexpr float glotPitchMax = 600;

constexpr float glotRdMin = 0.7;
constexpr float glotRdMax = 2.6;

constexpr float filterShiftMin = 0.8;
constexpr float filterShiftMax = 2.0;

void ContextManager::renderSynth(RenderingContext& rctx)
{
    int rw, rh;
    rctx.target->getSizeForRenderer(&rw, &rh);
 
    auto& font = FONT(primaryFont, uiFontSize - 3, rctx);

    int em = std::get<3>(font.queryTextSize("M"));

    int tx, ty, tw, th;

    int x = 10, x2;
    int y = 10;
    int padding = 5;
    int margin = 25;

    int sliderWidth = 350;
    int sliderKnobWidth = 16;

    std::array<int, 4> pos;

    std::stringstream ss;
    
    ss << "Master gain";
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x, y + 5, 1.0f, 1.0f, 1.0f);
    x2 = x + tw + margin;
    std::tie(pos[0], pos[1], pos[2], pos[3]) =
        rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth, false);
    rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth * synth.getMasterGain(), true);
    rctx.renderer->renderRoundedRect(x2 + sliderWidth * synth.getMasterGain() - sliderKnobWidth / 2, y - 2, sliderKnobWidth, em + 14, 0.5f, 0.5f, 0.5f, 1.0f);
    ss.str("");
    ss << std::round(synth.getMasterGain() * 100) << "%";
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x2 + 5 + sliderWidth / 2 - tw / 2, y + 5, 1.0f, 1.0f, 1.0f);

    posMasterGain = pos;

    y += em + 10 + margin;

    ss.str("");
    ss << "Noise gain";
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x, y + 5, 1.0f, 1.0f, 1.0f);
    x2 = x + tw + margin;
    std::tie(pos[0], pos[1], pos[2], pos[3]) =
        rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth, false);
    rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth * synth.getNoiseGain(), true);
    rctx.renderer->renderRoundedRect(x2 + sliderWidth * synth.getNoiseGain() - sliderKnobWidth / 2, y - 2, sliderKnobWidth, em + 14, 0.5f, 0.5f, 0.5f, 1.0f);
    ss.str("");
    ss << std::round(synth.getNoiseGain() * 100) << "%";
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x2 + 5 + sliderWidth / 2 - tw / 2, y + 5, 1.0f, 1.0f, 1.0f);

    posNoiseGain = pos;

    y += em + 10 + margin;

    ss.str("");
    ss << "Glottal source gain";
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x, y + 5, 1.0f, 1.0f, 1.0f);
    x2 = x + tw + margin;
    std::tie(pos[0], pos[1], pos[2], pos[3]) =
        rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth, false);
    rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth * synth.getGlotGain(), true);
    rctx.renderer->renderRoundedRect(x2 + sliderWidth * synth.getGlotGain() - sliderKnobWidth / 2, y - 2, sliderKnobWidth, em + 14, 0.5f, 0.5f, 0.5f, 1.0f);
    ss.str("");
    ss << std::round(synth.getGlotGain() * 100) << "%";
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x2 + 5 + sliderWidth / 2 - tw / 2, y + 5, 1.0f, 1.0f, 1.0f);
    
    posGlotGain = pos;
    
    y += em + 10 + margin;

    ss.str("");
    ss << "Glottal pitch";
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x, y + 5, 1.0f, 1.0f, 1.0f);
    x2 = x + tw + margin;
    std::tie(pos[0], pos[1], pos[2], pos[3]) =
        rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth, false);
    rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth * (synth.getGlotPitch() - glotPitchMin) / (glotPitchMax - glotPitchMin), true);
    rctx.renderer->renderRoundedRect(x2 + sliderWidth * (synth.getGlotPitch() - glotPitchMin) / (glotPitchMax - glotPitchMin) - sliderKnobWidth / 2, y - 2, sliderKnobWidth, em + 14, 0.5f, 0.5f, 0.5f, 1.0f);
    ss.str("");
    ss << std::round(synth.getGlotPitch()) << " Hz";
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x2 + 5 + sliderWidth / 2 - tw / 2, y + 5, 1.0f, 1.0f, 1.0f);
    
    posGlotPitch = pos;
    
    y += em + 10 + margin;

    ss.str("");
    ss << "Glottal Rd";
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x, y + 5, 1.0f, 1.0f, 1.0f);
    x2 = x + tw + margin;
    std::tie(pos[0], pos[1], pos[2], pos[3]) =
        rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth, false);
    rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth * (synth.getGlotRd() - glotRdMin) / (glotRdMax - glotRdMin), true);
    rctx.renderer->renderRoundedRect(x2 + sliderWidth * (synth.getGlotRd() - glotRdMin) / (glotRdMax - glotRdMin) - sliderKnobWidth / 2, y - 2, sliderKnobWidth, em + 14, 0.5f, 0.5f, 0.5f, 1.0f);
    ss.str("");
    ss << std::round(synth.getGlotRd() * 100) / 100;
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x2 + 5 + sliderWidth / 2 - tw / 2, y + 5, 1.0f, 1.0f, 1.0f);
    
    posGlotRd = pos;
    
    y += em + 10 + margin;

    ss.str("");
    ss << "Filter shift";
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x, y + 5, 1.0f, 1.0f, 1.0f);
    x2 = x + tw + margin;
    std::tie(pos[0], pos[1], pos[2], pos[3]) =
        rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth, false);
    rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth * (synth.getFilterShift() - filterShiftMin) / (filterShiftMax - filterShiftMin), true);
    rctx.renderer->renderRoundedRect(x2 + sliderWidth * (synth.getFilterShift() - filterShiftMin) / (filterShiftMax - filterShiftMin) - sliderKnobWidth / 2, y - 2, sliderKnobWidth, em + 14, 0.5f, 0.5f, 0.5f, 1.0f);
    ss.str("");
    ss << "x " << (std::round(synth.getFilterShift() * 10) / 10);
    std::tie(tx, ty, tw, th) = font.queryTextSize(ss.str());
    rctx.renderer->renderText(font, ss.str(), x2 + 5 + sliderWidth / 2 - tw / 2, y + 5, 1.0f, 1.0f, 1.0f);
    
    posFilterShift = pos;
    
    y += em + 10 + margin;
}

void ContextManager::eventSynth(RenderingContext& rctx)
{
    int tw, th;
    int rw, rh;
    rctx.target->getSize(&tw, &th);
    rctx.target->getSizeForRenderer(&rw, &rh);
    
    const auto [tx, ty] = rctx.target->getMousePosition();
    int x, y, w, h;

    std::array<int, 4> pos;

    if (rctx.target->isMousePressed(SDL_BUTTON_LEFT)) {
        pos = posMasterGain;
        x = (pos[0] * tw) / rw;
        y = (pos[1] * th) / rh;
        w = (pos[2] * tw) / rw;
        h = (pos[3] * th) / rh;
        if (tx >= x && tx <= x + w
                && ty >= y && ty <= y + h) {
            synth.setMasterGain((tx - x) / (float) w);
        }
        
        pos = posNoiseGain;
        x = (pos[0] * tw) / rw;
        y = (pos[1] * th) / rh;
        w = (pos[2] * tw) / rw;
        h = (pos[3] * th) / rh;
        if (tx >= x && tx <= x + w
                && ty >= y && ty <= y + h) {
            synth.setNoiseGain((tx - x) / (float) w);
        }
        
        pos = posGlotGain;
        x = (pos[0] * tw) / rw;
        y = (pos[1] * th) / rh;
        w = (pos[2] * tw) / rw;
        h = (pos[3] * th) / rh;
        if (tx >= x && tx <= x + w
                && ty >= y && ty <= y + h) {
            synth.setGlotGain((tx - x) / (float) w);
        }

        pos = posGlotPitch;
        x = (pos[0] * tw) / rw;
        y = (pos[1] * th) / rh;
        w = (pos[2] * tw) / rw;
        h = (pos[3] * th) / rh;
        if (tx >= x && tx <= x + w
                && ty >= y && ty <= y + h) {
            synth.setGlotPitch(glotPitchMin + (glotPitchMax - glotPitchMin) * (tx - x) / (float) w);
        }

        pos = posGlotRd;
        x = (pos[0] * tw) / rw;
        y = (pos[1] * th) / rh;
        w = (pos[2] * tw) / rw;
        h = (pos[3] * th) / rh;
        if (tx >= x && tx <= x + w
                && ty >= y && ty <= y + h) {
            synth.setGlotRd(glotRdMin + (glotRdMax - glotRdMin) * (tx - x) / (float) w);
        }

        pos = posFilterShift;
        x = (pos[0] * tw) / rw;
        y = (pos[1] * th) / rh;
        w = (pos[2] * tw) / rw;
        h = (pos[3] * th) / rh;
        if (tx >= x && tx <= x + w
                && ty >= y && ty <= y + h) {
            synth.setFilterShift(filterShiftMin + (filterShiftMax - filterShiftMin) * (tx - x) / (float) w);
        }

    }
}

#endif
