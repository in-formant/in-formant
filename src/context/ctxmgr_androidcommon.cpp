#include "contextmanager.h"

using namespace Main;

static std::array<int, 4> buttonPosPause;
static std::array<int, 4> buttonPosNoise;
static std::array<int, 4> buttonPosCursor;

static std::array<int, 4> buttonPosSpec;
static std::array<int, 4> buttonPosFFTspec;
static std::array<int, 4> buttonPosOscil;

void ContextManager::renderAndroidCommon(RenderingContext& rctx)
{
    int rw, rh;
    rctx.target->getSizeForRenderer(&rw, &rh);

    auto& font = primaryFont->with(uiFontSize - 4, rctx.target.get());

    int em = std::get<3>(font.queryTextSize("M"));
    int ty, tw;

    float g1 = 1.0f;
    float g2 = 0.7;
    int padding = 15;

    ty = 3 * rh / 5 - (6 * (em + 2 * padding + padding)) / 2;

    std::string str;
    float g;
    std::array<int, 4> pos;

    str = (isPaused ? "Resume" : "Pause");
    g = isPaused ? g2 : g1;
    tw = std::get<2>(font.queryTextSize(str));
    pos = {padding, ty, tw + 2 * padding, em + 2 * padding};
    rctx.renderer->renderRoundedRect(pos[0], pos[1], pos[2], pos[3], g, g, g, 0.9f);
    rctx.renderer->renderText(font, str, pos[0] + padding, pos[1] + padding, 0.0f, 0.0f, 0.0f);

    ty += em + 3 * padding;
    buttonPosPause = pos;

    str = (isNoiseOn ? "Noise off" : "Noise on");
    g = isNoiseOn ? g2 : g1;
    tw = std::get<2>(font.queryTextSize(str));
    pos = {padding, ty, tw + 2 * padding, em + 2 * padding};
    rctx.renderer->renderRoundedRect(pos[0], pos[1], pos[2], pos[3], g, g, g, 0.9f);
    rctx.renderer->renderText(font, str, pos[0] + padding, pos[1] + padding, 0.0f, 0.0f, 0.0f);

    ty += em + 3 * padding;
    buttonPosNoise = pos;

    str = (useFrameCursor ? "Hide cursor" : "Show cursor");
    g = useFrameCursor ? g2 : g1;
    tw = std::get<2>(font.queryTextSize(str));
    pos = {padding, ty, tw + 2 * padding, em + 2 * padding};
    rctx.renderer->renderRoundedRect(pos[0], pos[1], pos[2], pos[3], g, g, g, 0.9f);
    rctx.renderer->renderText(font, str, pos[0] + padding, pos[1] + padding, 0.0f, 0.0f, 0.0f);

    ty += em + 4 * padding;

    str = "Spectrogram";
    g = (selectedViewName == "Spectrogram" ? g2 : g1);
    tw = std::get<2>(font.queryTextSize(str));
    pos = {padding, ty, tw + 2 * padding, em + 2 * padding};
    rctx.renderer->renderRoundedRect(pos[0], pos[1], pos[2], pos[3], g, g, g, 0.9f);
    rctx.renderer->renderText(font, str, pos[0] + padding, pos[1] + padding, 0.0f, 0.0f, 0.0f);

    ty += em + 3 * padding;
    buttonPosSpec = pos;

    str = "FFT spectrum";
    g = (selectedViewName == "FFT spectrum" ? g2 : g1);
    tw = std::get<2>(font.queryTextSize(str));
    pos = {padding, ty, tw + 2 * padding, em + 2 * padding};
    rctx.renderer->renderRoundedRect(pos[0], pos[1], pos[2], pos[3], g, g, g, 0.9f);
    rctx.renderer->renderText(font, str, pos[0] + padding, pos[1] + padding, 0.0f, 0.0f, 0.0f);

    ty += em + 3 * padding;
    buttonPosFFTspec = pos;

    str = "Oscilloscope";
    g = (selectedViewName == "Oscilloscope" ? g2 : g1);
    tw = std::get<2>(font.queryTextSize(str));
    pos = {padding, ty, tw + 2 * padding, em + 2 * padding};
    rctx.renderer->renderRoundedRect(pos[0], pos[1], pos[2], pos[3], g, g, g, 0.9f);
    rctx.renderer->renderText(font, str, pos[0] + padding, pos[1] + padding, 0.0f, 0.0f, 0.0f);

    buttonPosOscil = pos;
}

void ContextManager::eventAndroidCommon(RenderingContext& rctx)
{
    int tw, th;
    int rw, rh;
    rctx.target->getSize(&tw, &th);
    rctx.target->getSizeForRenderer(&rw, &rh);

    const auto [tx, ty] = rctx.target->getMousePosition();
    int x, y, w, h;
    std::array<int, 4> pos;
    bool hover;

    pos = buttonPosPause;
    x = (pos[0] * tw) / rw;
    y = (pos[1] * th) / rh;
    w = (pos[2] * tw) / rw;
    h = (pos[3] * th) / rh;
    hover = (tx >= x && tx <= x + w
            && ty >= y && ty <= y + h);
    if (hover && rctx.target->isTouchPressed()) {
        isPaused = !isPaused;
    }

    pos = buttonPosNoise;
    x = (pos[0] * tw) / rw;
    y = (pos[1] * th) / rh;
    w = (pos[2] * tw) / rw;
    h = (pos[3] * th) / rh;
    hover = (tx >= x && tx <= x + w
            && ty >= y && ty <= y + h);
    if (hover && rctx.target->isTouchPressed()) {
        isNoiseOn = !isNoiseOn;
    }

    pos = buttonPosCursor;
    x = (pos[0] * tw) / rw;
    y = (pos[1] * th) / rh;
    w = (pos[2] * tw) / rw;
    h = (pos[3] * th) / rh;
    hover = (tx >= x && tx <= x + w
            && ty >= y && ty <= y + h);
    if (hover && rctx.target->isTouchPressed()) {
        useFrameCursor = !useFrameCursor;
    }
    
    pos = buttonPosSpec;
    x = (pos[0] * tw) / rw;
    y = (pos[1] * th) / rh;
    w = (pos[2] * tw) / rw;
    h = (pos[3] * th) / rh;
    hover = (tx >= x && tx <= x + w
            && ty >= y && ty <= y + h);
    if (hover && rctx.target->isTouchPressed()) {
        selectedViewName = "Spectrogram";
    } 
    
    pos = buttonPosFFTspec;
    x = (pos[0] * tw) / rw;
    y = (pos[1] * th) / rh;
    w = (pos[2] * tw) / rw;
    h = (pos[3] * th) / rh;
    hover = (tx >= x && tx <= x + w
            && ty >= y && ty <= y + h);
    if (hover && rctx.target->isTouchPressed()) {
        selectedViewName = "FFT spectrum";
    }

    pos = buttonPosOscil;
    x = (pos[0] * tw) / rw;
    y = (pos[1] * th) / rh;
    w = (pos[2] * tw) / rw;
    h = (pos[3] * th) / rh;
    hover = (tx >= x && tx <= x + w
            && ty >= y && ty <= y + h);
    if (hover && rctx.target->isTouchPressed()) {
        selectedViewName = "Oscilloscope";
    }
}
