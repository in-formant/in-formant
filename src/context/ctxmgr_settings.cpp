#include "contextmanager.h"

using namespace Main;

void ContextManager::initSettingsUI()
{
    mSettingFields = {
        { .labelText = "LP order: ",    .field = &ContextManager::linPredOrder},
        { .labelText = "Max formant: ", .field = &ContextManager::analysisMaxFrequency},
    };
}

void ContextManager::renderSettings(RenderingContext &rctx)
{
    auto& font = primaryFont->with(uiFontSize - 2, rctx.target.get());
  
    int x0 = 10;
    int y0 = 10;

    int defaultWidth = 80;

    for (auto& f : mSettingFields) { 
        const auto [tx, ty, tw, th] = font.queryTextSize(f.labelText);
        rctx.renderer->renderText(font, f.labelText, x0, y0 - th - ty + 5, 1.0f, 1.0f, 1.0f);
        
        const auto valstr = std::to_string(this->*f.field);
        std::tie(f.x, f.y, f.w, f.h) = rctx.renderer->renderInputBox(font, valstr, x0 + tw + 10, y0, defaultWidth, f.isFocused);

        y0 += f.h + 10;
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
                (this->*f.field)++;
                settingsChanged = true;
            }
            else if (rctx.target->isKeyPressed(SDL_SCANCODE_DOWN)) {
                (this->*f.field)--;
                settingsChanged = true;
            }
        }
    }
    
    if (settingsChanged) {
        updateNodeParameters();
    }
}

