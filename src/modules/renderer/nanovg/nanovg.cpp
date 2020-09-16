#include "nanovg.h"
#include <iostream>
#include <cmath>

using namespace Module::Renderer;

NanoVG::NanoVG()
    : AbstractBase(Type::NanoVG),
      mProvider(nullptr)
{
}

NanoVG::~NanoVG()
{
}

void NanoVG::setProvider(void *provider)
{
    mProvider = static_cast<NvgProvider *>(provider);
}

void NanoVG::initialize()
{
    vg = mProvider->createContext(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    if (vg == nullptr) {
        throw std::runtime_error("Renderer::NanoVG] Could not create NVGcontext object");
    }

    mSpectrogramCount = 0;
    mSpectrogramFb1 = mSpectrogramFb2 = nullptr;
}

void NanoVG::terminate()
{
    mProvider->deleteContext(vg);
}

void NanoVG::begin()
{ 
    getWindowSize(&mWidth, &mHeight);

    int drawableWidth, drawableHeight;
    getDrawableSize(&drawableWidth, &drawableHeight);

    mProvider->beforeBeginFrame();

    nvgBeginFrame(vg, mWidth, mHeight, (float) drawableWidth / (float) mWidth);

    nvgBeginPath(vg);
    nvgRect(vg, 0, 0, mWidth, mHeight);
    nvgFillColor(vg, nvgRGB(0, 0, 0));
    nvgFill(vg);
}

void NanoVG::end()
{
    nvgEndFrame(vg);

    mProvider->afterEndFrame();
}

void NanoVG::test()
{
}

void NanoVG::renderGraph(const GraphRenderData& graph, float thick, float r, float g, float b)
{
    float xi, yi;
    float xip, yip;
    
    nvgBeginPath(vg);
    nvgLineCap(vg, NVG_ROUND);
    nvgLineJoin(vg, NVG_ROUND);
    
    const auto& p0 = graph.front();
    const auto& pend = graph.back();

    xi = -0.9f;
    yi = p0.y / 10.0f;

    std::tie(xip, yip) = convertNormCoord(xi, yi);

    nvgMoveTo(vg, xip, yip);

    for (int i = 1; i < graph.size(); ++i) {
        const auto& p = graph[i];

        xi = 0.9 * (2.0f * (p.x - p0.x) / (pend.x - p0.x) - 1.0f); 
        yi = p.y / 10.0f;

        std::tie(xip, yip) = convertNormCoord(xi, yi);

        nvgLineTo(vg, xip, yip);
    }

    nvgStrokeColor(vg, nvgRGBf(r, g, b));
    nvgStrokeWidth(vg, thick);
    nvgStroke(vg);
}

void NanoVG::renderSpectrogram(const SpectrogramRenderData& slice, int count)
{
    constexpr int imageHeight = 2048;

    if (mSpectrogramCount != count) {
        if (mSpectrogramFb1 != nullptr) {
            mProvider->deleteFramebuffer(mSpectrogramFb1);
            mProvider->deleteFramebuffer(mSpectrogramFb2);
        }
        mSpectrogramCount = count;
        mSpectrogramFb1 = mProvider->createFramebuffer(vg, count, imageHeight, 0);
        mSpectrogramIm1 = mProvider->framebufferImage(mSpectrogramFb1);
        mSpectrogramFb2 = mProvider->createFramebuffer(vg, count, imageHeight, 0);
        mSpectrogramIm2 = mProvider->framebufferImage(mSpectrogramFb2);
    }

    nvgCancelFrame(vg);

    mProvider->bindFramebuffer(mSpectrogramFb2);

    nvgBeginFrame(vg, count, imageHeight, 1.0f); 
    nvgSave(vg);

    nvgResetTransform(vg);
    nvgTranslate(vg, -1, 0);

    nvgBeginPath(vg);
    nvgRect(vg,
        0,
        0,
        count,
        imageHeight);
    nvgFillPaint(vg,
            nvgImagePattern(
                vg,
                0, 0, count, imageHeight,
                0.0f, mSpectrogramIm1, 1.0f));
    nvgFill(vg);
   
    nvgRestore(vg);
    nvgSave(vg);

    float xp = (count - 1);

    if (!slice.empty()) {
        float frequency = slice[0].frequency;
        float intensity = slice[0].intensity;

        float y = frequencyToCoordinate(slice[0].frequency);

        float gain = intensity > 1e-10f ? 20.0f * log10(intensity) : -1e6f;
        float r, g, b;
        gainToColor(gain, &r, &g, &b);

        float yp = imageHeight - (y + 1.0f) / 2.0f * imageHeight;

        for (const auto& point : slice) {
            float frequency = point.frequency;
            float intensity = point.intensity;

            float y2 = frequencyToCoordinate(frequency);

            float nextYp = imageHeight - (y2 + 1.0f) / 2.0f * imageHeight;
           
            if (std::abs(std::round(yp) - std::round(nextYp)) >= 1) {
                float gain = intensity > 1e-10f ? 20.0f * log10(intensity) : -1e6f;
                float nr, ng, nb;
                gainToColor(gain, &nr, &ng, &nb);

                nvgBeginPath(vg);
                nvgMoveTo(vg, xp, yp);
                nvgLineTo(vg, xp, nextYp);
                nvgStrokePaint(vg,
                        nvgLinearGradient(
                            vg,
                            xp, yp, xp, nextYp,
                            nvgRGBf(r, g, b),
                            nvgRGBf(nr, ng, nb)));
                nvgStrokeWidth(vg, 1.0f);
                nvgStroke(vg);

                yp = nextYp;
                r = nr; g = ng; b = nb;
            }
        }
    }

    nvgRestore(vg);
    nvgEndFrame(vg);

    std::swap(mSpectrogramFb1, mSpectrogramFb2);
    std::swap(mSpectrogramIm1, mSpectrogramIm2);

    mProvider->bindFramebuffer(nullptr);

    begin();
    nvgSave(vg);

    nvgResetTransform(vg);

    nvgBeginPath(vg);
    nvgRect(vg,
        0,
        0,
        mWidth,
        mHeight);
    nvgFillPaint(vg,
            nvgImagePattern(
                vg,
                0, 0, mWidth, mHeight,
                0.0f, mSpectrogramIm1, 1.0f));
    nvgFill(vg);

    nvgRestore(vg);
}

void NanoVG::renderFrequencyTrack(const FrequencyTrackRenderData& track, float thick, float r, float g, float b)
{
    nvgBeginPath(vg);
    nvgLineCap(vg, NVG_ROUND);
    nvgLineJoin(vg, NVG_ROUND);
    
    float xstep = (float) mWidth / (float) track.size();

    for (int i = 0; i < track.size(); ++i) {
        const auto& point = track[i];

        if (point.has_value()) {
            float xp = i * xstep;

            float y = frequencyToCoordinate(point.value());
            float yp = mHeight - (y + 1.0f) / 2.0f * mHeight;

            if (i == 0 || !track[i - 1].has_value()) {
                nvgMoveTo(vg, xp, yp);
            }
            else {
                nvgLineTo(vg, xp, yp);
            }
        }
    }

    nvgStrokeColor(vg, nvgRGBf(r, g, b));
    nvgStrokeWidth(vg, thick);
    nvgStroke(vg);
}

void NanoVG::renderFrequencyScaleBar(Module::Freetype::Font& majorFont, Module::Freetype::Font& minorFont)
{
    FrequencyScale scale = getParameters()->getFrequencyScale();
    float min = getParameters()->getMinFrequency();
    float max = getParameters()->getMaxFrequency();

    constexpr int   tickLens[2]   = {5, 3};
    constexpr float tickThicks[2] = {3, 2};

    if (scale == FrequencyScale::Linear) {

        int lo = 100 * (((int) min / 100) - 1);
        int hi = 100 * (((int) max / 100) + 1);

        for (int val = lo; val <= hi; val += 100) {
            if (val >= min && val < max) {
                float y = frequencyToCoordinate(val);
                float yp = mHeight - (y + 1.0f) / 2.0f * mHeight;

                bool majorTick = (val % 1000 == 0);
                int tickLen                       = tickLens[!majorTick];
                float tickThick                   = tickThicks[!majorTick];
                Module::Freetype::Font *labelFont = majorTick ? &majorFont : &minorFont;

                nvgBeginPath(vg);
                nvgLineCap(vg, NVG_SQUARE);
                nvgMoveTo(vg, mWidth - 1, yp);
                nvgLineTo(vg, mWidth - 1 - tickLen, yp);
                nvgStrokeColor(vg, nvgRGBf(1, 1, 1));
                nvgStrokeWidth(vg, tickThick);
                nvgStroke(vg);

                if (majorTick || val % 500 == 0) {
                    const auto valstr = std::to_string((int) val);
                    const auto [tx, ty, tw, th] = labelFont->queryTextSize(valstr + "  ");
                    renderText(*labelFont, valstr, mWidth - 1 - tickLens[0] - tw, yp + ty + th / 2, 1, 1, 1);
                }
            }
        }
    }
    else {
        float loLog = log10(min);
        float hiLog = log10(max);
        int loDecade = (int) floor(loLog);

        float val;
        float startDecade = pow(10.0f, loDecade);
            
        float decade = startDecade;
        float delta = hiLog - loLog, steps = fabs(delta);
        float step = delta >= 0 ? 10 : 0.1;
        float rMin = std::min(min, max), rMax = std::max(min, max);
        float start, end, mstep;
        
        decade = startDecade;
        if (delta > 0) {
            start = 10;
            end = 100;
            mstep = 1;
        }
        else {
            start = 100;
            end = 10;
            mstep = -1;
        }
        steps++;
        for (int i = 0; i <= steps; ++i) {
            for (int f = start; f != (int) end; f += mstep) {
                val = decade * f / 10;
                if (val >= rMin && val < rMax) {
                    float y = frequencyToCoordinate(val);
                    float yp = mHeight - (y + 1.0f) / 2.0f * mHeight;

                    bool majorTick = ((int) (f / 10.0f) == f / 10.0f);

                    int tickLen                       = tickLens[!majorTick];
                    float tickThick                   = tickThicks[!majorTick];
                    Module::Freetype::Font *labelFont = majorTick ? &majorFont : &minorFont;

                    nvgBeginPath(vg);
                    nvgLineCap(vg, NVG_SQUARE);
                    nvgMoveTo(vg, mWidth - 1, yp);
                    nvgLineTo(vg, mWidth - 1 - tickLen, yp);
                    nvgStrokeColor(vg, nvgRGBf(1, 1, 1));
                    nvgStrokeWidth(vg, tickThick);
                    nvgStroke(vg);
                  
                    bool shouldRenderLabel;

                    if (scale == FrequencyScale::Mel) {
                        shouldRenderLabel =
                               ( 500 <= val && fmod(val, 500) == 0)
                            || ( 100 <= val && val <  500 && fmod(val,  100) == 0)
                            || (  10 == val);
                    }
                    else if (scale == FrequencyScale::Logarithmic) {
                        shouldRenderLabel =
                               (1000 <= val && fmod(val, 1000) == 0)
                            || ( 100 <= val && val < 1000 && fmod(val, 100) == 0)
                            || (  10 <= val && val <  100 && fmod(val,  10) == 0)
                            || (   1 == val);
                    }

                    if (shouldRenderLabel) {
                        const auto valstr = std::to_string((int) val);
                        const auto [tx, ty, tw, th] = labelFont->queryTextSize(valstr + "  ");
                        renderText(*labelFont, valstr, mWidth - 1 - tickLens[0] - tw, yp + ty + th / 2, 1, 1, 1);
                    }
                }
            }
            decade *= step;
        }
    }
}

void NanoVG::renderText(Module::Freetype::Font& font, const std::string& text, int x0, int y0, float r, float g, float b)
{
    if (!font.hasAttachment()) {
        font.setAttachment(new NvgUtils::FontAttachment(vg, font));
    }
    auto fa = font.getAttachment<NvgUtils::FontAttachment>();

    Module::Freetype::TextRenderData textRenderData = font.prepareTextRender(text);
    const auto [_1, _2, textWidth, textHeight] = font.queryTextSize(text);
     
    y0 = y0 + textHeight;

    for (const auto& glyphRenderData : textRenderData.glyphs) {
        int x = x0 + glyphRenderData.left;
        int y = y0 - glyphRenderData.top;

        nvgBeginPath(vg);
        nvgRect(vg,
            x,
            y,
            glyphRenderData.width,
            glyphRenderData.height);

        int image = fa->getImageFor(glyphRenderData.character);
        NVGpaint paint = nvgImagePattern(
                vg, x, y, glyphRenderData.width, glyphRenderData.height, 0.0f, image, 1.0f);

        paint.innerColor = nvgRGBf(r, g, b);

        nvgFillPaint(vg, paint);
        nvgFill(vg);
        
        x0 += glyphRenderData.advanceX >> 6;
    }
}

std::tuple<float, float, float, float> NanoVG::renderInputBox(Module::Freetype::Font& font, const std::string& content, int x, int y, int w, bool isFocused)
{
    int h = std::get<3>(font.queryTextSize("M")) + 10;
    
    NVGcolor startClr = isFocused ? nvgRGBA( 70, 102, 255, 128) : nvgRGBA(255, 255, 255,  32);
    NVGcolor endClr   = isFocused ? nvgRGBA( 32,  32,  32,  32) : nvgRGBA( 32,  32,  32,  32);

    NVGpaint bg;
    bg = nvgBoxGradient(vg, x+1,y+1+1.5f, w-2,h-2, 3,4, startClr, endClr);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x+1,y+1, w-2, h-2, 4-1);
    nvgFillPaint(vg, bg);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x+0.5f,y+0.5f, w-1,h-1, 4-0.5f);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,48));
    nvgStroke(vg);

    renderText(font, content, x + 5, y + 5, 1.0f, 1.0f, 1.0f);

    return {
        (float) x,
        (float) y,
        (float) w,
        (float) h,
    };
}

std::pair<float, float> NanoVG::convertNormCoord(float x, float y)
{
    return {
        (x + 1.0f) / 2.0f * mWidth,
        mHeight - (y + 1.0f) / 2.0f * mHeight,
    };
}
