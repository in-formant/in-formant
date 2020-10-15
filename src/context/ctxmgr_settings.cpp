#include "contextmanager.h"

using namespace Main;

void ContextManager::initSettingsUI()
{
    mSettingFields = {
        {
            .labelText = "Max. formant frequency: ",
            .min = 3000, .max = 7000,
            .value = [this]() { return analysisMaxFrequency; },
            .update = [this](float v) { analysisMaxFrequency = v; linPredOrder = std::round(v / 500) + linPredOrderOffset; }, 
            .barText = [this]() { return std::to_string((int) analysisMaxFrequency) + " Hz / LPO: " + std::to_string(linPredOrder + linPredOrderOffset); },
        },
        {
            .labelText = "LP order offset: ",
            .min = -2, .max = 2,
            .value = [this]() { return linPredOrderOffset; },
            .update = [this](float v) { linPredOrderOffset = std::round(v); },
            .barText = [this]() { return (linPredOrderOffset == 0 ? "" :
                                            linPredOrderOffset > 0 ? "+" : "-")
                                    + std::to_string(std::abs(linPredOrderOffset)); }
        },
        {
            .labelText = "Min. spec. frequency: ",
            .min = 1, .max = 1000,
            .value = [this]() { return viewMinFrequency; },
            .update = [this](float v) { viewMinFrequency = v; },
            .barText = [this]() { return std::to_string((int) viewMinFrequency) + " Hz"; },
        },
        { 
            .labelText = "Max. spec. frequency: ",
            .min = 4000, .max = 12000,
            .value = [this]() { return viewMaxFrequency; },
            .update = [this](float v) { viewMaxFrequency = v; },
            .barText = [this]() { return std::to_string((int) viewMaxFrequency) + " Hz"; },
        },
        {
            .labelText = "Min. spec. gain: ",
            .min = -200, .max = -15,
            .value = [this]() { return viewMinGain; },
            .update = [this](float v) { viewMinGain = v; },
            .barText = [this]() { return std::to_string((int) viewMinGain) + " dB"; },
        },
        {
            .labelText = "Max. spec. gain: ",
            .min = -10, .max = 80,
            .value = [this]() { return viewMaxGain; },
            .update = [this](float v) { viewMaxGain = v; },
            .barText = [this]() { return std::to_string((int) viewMaxGain) + " dB"; },
        },
    };
}

void ContextManager::renderSettings(RenderingContext &rctx)
{
    auto& font = FONT(primaryFont, uiFontSize - 4, rctx);
  
    int em = std::get<3>(font.queryTextSize("M"));

    int x = 10, x2;
    int y = 10;
    int padding = 5;
    int margin = 25;

    int sliderWidth = 300;
    int sliderKnobWidth = 16;

    for (auto& f : mSettingFields) { 
        const auto [tx, ty, tw, th] = font.queryTextSize(f.labelText);
        rctx.renderer->renderText(font, f.labelText, x, y + padding, 1.0f, 1.0f, 1.0f);
       
        const float val = f.value();
        const auto valstr = f.barText();

        const float frac = (val - f.min) / (f.max - f.min);

        x2 = x + tw + margin;

        std::tie(f.x, f.y, f.w, f.h) = rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth, false);
        rctx.renderer->renderInputBox(font, "", x2, y, sliderWidth * frac, true);
        rctx.renderer->renderRoundedRect(x2 + sliderWidth * frac - sliderKnobWidth / 2, y - 2, sliderKnobWidth, em + 14, 0.5f, 0.5f, 0.5f, 1.0f);

        int vsw = std::get<2>(font.queryTextSize(valstr));
        rctx.renderer->renderText(font, valstr, x2 + padding + sliderWidth / 2 - vsw / 2, y + padding, 1.0f, 1.0f, 1.0f); 
    
        y += em + 2 * padding + margin;
    }
}

void ContextManager::eventSettings(RenderingContext &rctx)
{
    const auto [mx, my] = rctx.target->getMousePosition();

    int tw, th;
    int rw, rh;
    rctx.target->getSize(&tw, &th);
    rctx.target->getSizeForRenderer(&rw, &rh);
   
    bool settingsChanged = false;

    if (rctx.target->isMousePressed(SDL_BUTTON_LEFT)) {
        for (auto& f : mSettingFields) {
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
    
    if (settingsChanged) {
        updateNodeParameters();
        updateAllRendererParameters();
    }
}

