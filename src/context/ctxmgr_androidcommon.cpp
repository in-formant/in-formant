#include "contextmanager.h"

using namespace Main;

struct ButtonEntry {
    int x, y, w, h;
    std::function<std::string()> icon;
    std::function<float()> gray;
    std::function<void()> action;
};

static std::vector<ButtonEntry> buttonsTop;
static std::vector<ButtonEntry> buttonsBottom;

void ContextManager::initAndroidUI()
{
    buttonsTop = {
        {
            .icon = [this]() { return isPaused ? "play.svg" : "pause.svg"; },
            .gray = [this]() { return isPaused ? 0.7f : 1.0f; },
            .action = [this]() { isPaused = !isPaused; },
        },
        {
            .icon = [this]() { return isNoiseOn ? "play-circle.svg" : "pause-circle.svg"; },
            .gray = [this]() { return isNoiseOn ? 0.7f : 1.0f; },
            .action = [this]() { isNoiseOn = !isNoiseOn; },
        },
        {
            .icon = [this]() { return "add.svg"; },
            .gray = [this]() { return useFrameCursor ? 0.7f : 1.0f; },
            .action = [this]() { useFrameCursor = !useFrameCursor; },
        },
    };
    
    buttonsBottom = {
        {
            .icon = [this]() { return "analytics.svg"; },
            .gray = [this]() { return (selectedViewName == "Spectrogram") ? 0.7f : 1.0f; },
            .action = [this]() { selectedViewName = "Spectrogram"; },
        },
        {
            .icon = [this]() { return "stats-chart.svg"; },
            .gray = [this]() { return (selectedViewName == "FFT spectrum") ? 0.7f : 1.0f; },
            .action = [this]() { selectedViewName = "FFT spectrum"; },
        },
        {
            .icon = [this]() { return "barcode.svg"; },
            .gray = [this]() { return (selectedViewName == "Oscilloscope") ? 0.7f : 1.0f; },
            .action = [this]() { selectedViewName = "Oscilloscope"; },
        },
    };
}

void ContextManager::renderAndroidCommon(RenderingContext& rctx)
{
    if (selectedViewName != "Spectrogram") {
        scrollSpectrogram(rctx);
    }

    int rw, rh;
    rctx.target->getSizeForRenderer(&rw, &rh);
    
    auto& font = FONT(primaryFont, uiFontSize + 3, rctx);

    int em = std::get<3>(font.queryTextSize("M"));
    int cx1, cy1, cx2, cy2, dx, dy;

    float g1 = 1.0f;
    float g2 = 0.7;
    int padding = 5;
    int margin = 20;

    if (rh >= rw) {
        dx = 0;
        dy = em + 2 * padding + margin;
        cx1 = em / 2 + padding + margin / 2;
        cx2 = cx1 + dy;
        cy1 = cy2 = 3 * rh / 5;
    }
    else {
        dx = em + 2 * padding + margin;
        dy = 0;
        cx1 = cx2 = rw / 2;
        cy1 = em / 2 + padding + margin / 2;
        cy2 = cy1 + dx;
    }

    float hdpi, vdpi, ddpi;
    rctx.target->getDisplayDPI(&hdpi, &vdpi, &ddpi);
    float dpi = (hdpi + vdpi) / 2.0f;

    std::string str;
    float g;
    std::array<int, 4> pos;

    float x, y;

    x = cx1 - buttonsTop.size() * dx / 2;
    y = cy1 - buttonsTop.size() * dy / 2;

    for (auto& b : buttonsTop) {
        std::string icon = b.icon();
        float gray = b.gray();

        b.x = x;
        b.y = y;
        b.w = em + 2 * padding;
        b.h = em + 2 * padding;
        rctx.renderer->renderRoundedRect(b.x, b.y, b.w, b.h, gray, gray, gray, 0.9f);
        rctx.renderer->renderSVG(icon, dpi, b.x + padding, b.y + padding, em, em);

        x += dx;
        y += dy;
    }

    x = cx2 - buttonsTop.size() * dx / 2;
    y = cy2 - buttonsTop.size() * dy / 2;

    for (auto& b : buttonsBottom) {
        std::string icon = b.icon();
        float gray = b.gray();

        b.x = x;
        b.y = y;
        b.w = em + 2 * padding;
        b.h = em + 2 * padding;
        rctx.renderer->renderRoundedRect(b.x, b.y, b.w, b.h, gray, gray, gray, 0.9f);
        rctx.renderer->renderSVG(icon, dpi, b.x + padding, b.y + padding, em, em);

        x += dx;
        y += dy;
    }
}

void ContextManager::eventAndroidCommon(RenderingContext& rctx)
{
    int tw, th;
    int rw, rh;
    rctx.target->getSize(&tw, &th);
    rctx.target->getSizeForRenderer(&rw, &rh);

    const auto [tx, ty] = rctx.target->getMousePosition();
    int x, y, w, h;

    if (rctx.target->isTouchPressed()) {
        for (const auto& b : buttonsTop) {
            int x = (b.x * tw) / rw;
            int y = (b.y * th) / rh;
            int w = (b.w * tw) / rw;
            int h = (b.h * th) / rh;

            if (tx >= x && tx <= x + w
                    && ty >= y && ty <= y + h) {
                b.action();
            }
        }

        for (const auto& b : buttonsBottom) {
            int x = (b.x * tw) / rw;
            int y = (b.y * th) / rh;
            int w = (b.w * tw) / rw;
            int h = (b.h * th) / rh;

            if (tx >= x && tx <= x + w
                    && ty >= y && ty <= y + h) {
                b.action();
            }
        }
    }
}
