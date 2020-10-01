#include "contextmanager.h"

using namespace Main;

void ContextManager::initSettingsUI()
{
    mSettingFields = {
        { .labelText = "LP order: ", .field = &ContextManager::linPredOrder,         
            .delta = 1, .min = 4, .max = 30 },
        { .labelText = "Max. formant frequency: ", .field = &ContextManager::analysisMaxFrequency,
            .delta = 100, .min = 2500, .max = 8000 },
        { .labelText = "Min. displayed frequency: ", .field = &ContextManager::viewMinFrequency,
            .delta = 10, .min = 1, .max = 1000 },
        { .labelText = "Max. displayed frequency: ", .field = &ContextManager::viewMaxFrequency,
            .delta = 100, .min = 4500, .max = 8000 },
        { .labelText = "Min. displayed gain: ", .field = &ContextManager::viewMinGain,
            .delta = 1, .min = -200, .max = -15 },
        { .labelText = "Max. displayed gain: ", .field = &ContextManager::viewMaxGain,
            .delta = 1, .min = -10, .max = 80 },
    };
}

void ContextManager::renderSettings(RenderingContext &rctx)
{
    auto& font = FONT(primaryFont, uiFontSize - 3, rctx);
  
    int x0 = 10;
    int y0 = 10;

    int defaultWidth = 80;

    for (auto& f : mSettingFields) { 
        const auto [tx, ty, tw, th] = font.queryTextSize(f.labelText);
        rctx.renderer->renderText(font, f.labelText, x0, y0 - th - ty + 5, 1.0f, 1.0f, 1.0f);
        
        const auto valstr = std::to_string(this->*f.field);
        std::tie(f.x, f.y, f.w, f.h) = rctx.renderer->renderInputBox(font, valstr, x0 + tw + 15, y0, defaultWidth, f.isFocused);

        y0 += f.h + 15;
    }
}

void ContextManager::eventSettings(RenderingContext &rctx)
{
    const auto [mx, my] = rctx.target->getMousePosition();
    
    bool settingsChanged = false;

    for (auto& f : mSettingFields) {
        if (mx >= f.x && mx < f.x + f.w
                && my >= f.y && my < f.y + f.h) {
            f.isFocused = true;
        }
        else {
            f.isFocused = false;
        }

        if (f.isFocused) {
            if (rctx.target->isKeyPressed(SDL_SCANCODE_UP)) {
                (this->*f.field) = std::min(std::max((this->*f.field / f.delta + 1) * f.delta, f.min), f.max);
                settingsChanged = true;
            }
            else if (rctx.target->isKeyPressed(SDL_SCANCODE_DOWN)) {
                (this->*f.field) = std::min(std::max((this->*f.field / f.delta - 1) * f.delta, f.min), f.max);
                settingsChanged = true;
            }
        }
    }
    
    if (settingsChanged) {
        updateNodeParameters();
        updateAllRendererParameters();
    }
}

