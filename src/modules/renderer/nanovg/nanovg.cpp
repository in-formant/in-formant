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
    for (const auto& [path, image] : mSvgImages) {
        nvgDeleteImage(vg, image);
    }

    if (mSpectrogramFb1 != nullptr) {
        mProvider->deleteFramebuffer(mSpectrogramFb1);
        mProvider->deleteFramebuffer(mSpectrogramFb2);
    }

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

void NanoVG::renderGraph(const GraphRenderData& graph, float pmin, float pmax, float thick, float r, float g, float b)
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

        xi = 0.9 * (2.0f * (p.x - pmin) / (pmax - pmin) - 1.0f); 
        yi = p.y / 10.0f;

        std::tie(xip, yip) = convertNormCoord(xi, yi);

        nvgLineTo(vg, xip, yip);
    }

    nvgStrokeColor(vg, nvgRGBf(r, g, b));
    nvgStrokeWidth(vg, thick);
    nvgStroke(vg);
}

void NanoVG::scrollSpectrogram(const SpectrogramRenderData& slice, int count)
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

        float gain = intensity > 1e-10f ? 20.0f * log10f(intensity) : -1e6f;
        float r, g, b;
        gainToColor(gain, &r, &g, &b);

        float yp = imageHeight - (y + 1.0f) / 2.0f * imageHeight;

        for (const auto& point : slice) {
            float frequency = point.frequency;
            float intensity = point.intensity;

            float y2 = frequencyToCoordinate(frequency);

            float nextYp = imageHeight - (y2 + 1.0f) / 2.0f * imageHeight;
           
            if (std::abs(std::round(yp) - std::round(nextYp)) >= 1) {
                float gain = intensity > 1e-10f ? 20.0f * log10f(intensity) : -1e6f;
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
}

void NanoVG::renderSpectrogram()
{ 
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

    nvgStrokeWidth(vg, thick + 1.0f);
    nvgStrokeColor(vg, nvgRGBAf(r, g, b, 1.0f));
    nvgStroke(vg);

    nvgStrokeWidth(vg, 2.0f);
    nvgStrokeColor(vg, nvgRGBAf(0, 0, 0, 1.0f));
    nvgStroke(vg);
}

void NanoVG::renderFormantTrack(const FormantTrackRenderData& track, float thick, float r, float g, float b)
{
    float xstep = (float) mWidth / (float) track.size();

    for (int i = 0; i < track.size(); ++i) {
        const auto& point = track[i];

        if (point.has_value()) {
            const auto& formant = point.value();

            float xp = i * xstep;

            float y1 = frequencyToCoordinate(formant.frequency - formant.bandwidth / 4.0f);
            float y1p = mHeight - (y1 + 1.0f) / 2.0f * mHeight;

            float y2 = frequencyToCoordinate(formant.frequency + formant.bandwidth / 4.0f);
            float y2p = mHeight - (y2 + 1.0f) / 2.0f * mHeight;

            float cx = xp + xstep / 2.0f;
            float cy = (y1p + y2p) / 2.0f;
            float rx = xstep + 0.5f;
            float ry = std::max(fabsf(y1p - y2p) / 2.0f, 4.0f);

            nvgBeginPath(vg);
            nvgEllipse(vg, cx, cy, rx, ry);
            nvgFillPaint(vg,
                    nvgRadialGradient(vg, cx, cy, ry / 4, ry,
                        nvgRGBf(r, g, b), nvgRGBAf(r, g, b, 1.0f / 1800.0f)));
            nvgFill(vg);
        }
    }

    nvgBeginPath(vg);
    nvgLineCap(vg, NVG_ROUND);
    nvgLineJoin(vg, NVG_ROUND);

    float prevYp;

    for (int i = 0; i < track.size(); ++i) {
        const auto& point = track[i];

        if (point.has_value()) {
            const auto& formant = point.value();

            float xp = i * xstep;

            float y = frequencyToCoordinate(formant.frequency);
            float yp = mHeight - (y + 1.0f) / 2.0f * mHeight;

            if (i == 0 || !track[i - 1].has_value()) {
                nvgMoveTo(vg, xp, yp);
            }
            else {
                float xmid = xp - xstep / 2.0f;
                float ymid = (prevYp + yp) / 2.0f;

                float cx1 = (xmid + xp - xstep) / 2;
                float cx2 = (xmid + xp + xstep) / 2;

                nvgQuadTo(vg, cx1, prevYp, xmid, ymid);
                nvgQuadTo(vg, cx2, yp, xp, yp);
            }

            prevYp = yp;
        }
    }

    nvgStrokeWidth(vg, thick + 1.0f);
    nvgStrokeColor(vg, nvgRGBAf(1, 1, 1, 0.5f));
    nvgStroke(vg);

    nvgStrokeWidth(vg, thick - 1.0f);
    nvgStrokeColor(vg, nvgRGBAf(0, 0, 0, 1.0f));
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
        float loLog = log10f(min);
        float hiLog = log10f(max);
        int loDecade = (int) floorf(loLog);

        float val;
        float startDecade = powf(10.0f, loDecade);
            
        float decade = startDecade;
        float delta = hiLog - loLog, steps = fabsf(delta);
        float step = delta >= 0 ? 10 : 0.1;
        float rMin = std::min(min, max), rMax = std::max(min, max);
        float start, end, sstep, mstep;
        
        decade = startDecade;
        if (delta > 0) {
            start = 10;
            end = 100;
            sstep = 1;
        }
        else {
            start = 100;
            end = 10;
            sstep = -1;
        }
        steps++;
        for (int i = 0; i <= steps; ++i) {
            if (startDecade <= 10) {
                if (decade < 100) {
                    mstep = 100 * sstep;
                }
                else if (decade < 1000) {
                    mstep = 10 * sstep;
                }
                else {
                    mstep = sstep;
                }
            }
            else {
                if (decade < 1000) {
                    mstep = 10 * sstep;
                }
                else {
                    mstep = sstep;
                }
            }

            for (int f = start; f <= (int) end; f += mstep) {
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
                        int ival = std::round(val);
                        shouldRenderLabel =
                               ( 500 <= ival && ival % 500 == 0)
                            || ( 100 <= ival && ival < 500 && ival % 100 == 0)
                            || (  10 == ival);
                    }
                    else if (scale == FrequencyScale::Logarithmic) {
                        int ival = std::round(val);
                        shouldRenderLabel =
                               (1000 <= ival && ival % 1000 == 0)
                            || ( 100 <= ival && ival < 1000 && ival % 100 == 0)
                            || (  10 <= ival && ival <  100 && ival %  10 == 0)
                            || (   1 == ival);
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

float NanoVG::renderFrequencyCursor(float mx, float my)
{
    float yp = my * mHeight;
    
    nvgBeginPath(vg);
    nvgMoveTo(vg, 0, yp);
    nvgLineTo(vg, mWidth, yp);
    nvgStrokeColor(vg, nvgRGBf(1, 1, 1));
    nvgStrokeWidth(vg, 3.0f);
    nvgStroke(vg);
    
    float y = 2.0f * (0.5f - my);

    return coordinateToFrequency(y);
}

int NanoVG::renderFrameCursor(float mx, float my, int count)
{
    int frame = std::min<int>(std::max<int>(std::round(mx * count), 0), count - 1);
    float xp = (float) (mWidth * frame) / (float) count;

    nvgBeginPath(vg);
    nvgMoveTo(vg, xp, 0);
    nvgLineTo(vg, xp, mHeight);
    nvgStrokeColor(vg, nvgRGBf(1, 1, 1));
    nvgStrokeWidth(vg, 3.0f);
    nvgStroke(vg);

    return frame;
}

void NanoVG::renderRoundedRect(float x, float y, float w, float h, float r, float g, float b, float a)
{
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, w, h, 10.0);
    nvgFillColor(vg, nvgRGBAf(r, g, b, a));
    nvgFill(vg);
}

void NanoVG::renderSVG(const std::string& path, float dpi, float x, float y, float w, float h)
{
    int image;

    auto it = mSvgImages.find(path);
    if (it != mSvgImages.end()) {
        image = it->second;
    }
    else {
        NSVGimage *svgmg = nsvgParseFromFile(path.c_str(), "px", dpi);
        int iw = svgmg->width;
        int ih = svgmg->height;

        NSVGrasterizer *rast = nsvgCreateRasterizer();
        
        std::vector<unsigned char> data(iw * ih * 4);
        nsvgRasterize(rast, svgmg, 0, 0, 1, data.data(), iw, ih, iw * 4);

        nsvgDeleteRasterizer(rast);

        image = nvgCreateImageRGBA(vg, iw, ih, 0, data.data());

        mSvgImages[path] = image;
        mSvgImageData[path] = std::move(data);
    }

    nvgBeginPath(vg);
    nvgRect(vg, x, y, w, h);
    nvgFillPaint(vg,
            nvgImagePattern(
                vg, x, y, w, h, 0.0f, image, 1.0f));
    nvgFill(vg);
}

void NanoVG::renderText(Module::Freetype::Font& font, const std::string& text, int x0, int y0, float r, float g, float b)
{
    if (!font.hasAttachment()) {
        font.setAttachment(new NvgUtils::FontAttachment(vg, font), [](void *p) { delete (NvgUtils::FontAttachment *) p; });
    }
    auto fa = font.getAttachment<NvgUtils::FontAttachment>();

    Module::Freetype::TextRenderData textRenderData = font.prepareTextRender(text);
    const auto [tx, ty, tw, th] = font.queryTextSize(text);
     
    y0 = y0 - ty;

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

        paint.innerColor = nvgRGBAf(r, g, b, 1.0f);

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

uintptr_t NanoVG::getContextNumber()
{
    return (uintptr_t) (vg);
}
