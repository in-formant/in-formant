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

    nvgBeginFrame(vg, mWidth, mHeight, (double) drawableWidth / (double) mWidth);

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

void NanoVG::renderGraph(const GraphRenderData& graph, double pmin, double pmax, double thick, double r, double g, double b)
{
    double xi, yi;
    double xip, yip;
    
    nvgBeginPath(vg);
    nvgLineCap(vg, NVG_ROUND);
    nvgLineJoin(vg, NVG_ROUND);
    
    const auto& p0 = graph.front();
    const auto& pend = graph.back();

    xi = -0.9;
    yi = p0.y / 10.0;

    std::tie(xip, yip) = convertNormCoord(xi, yi);

    nvgMoveTo(vg, xip, yip);

    for (int i = 1; i < graph.size(); ++i) {
        const auto& p = graph[i];

        xi = 0.9 * (2.0 * (p.x - pmin) / (pmax - pmin) - 1.0); 
        yi = p.y / 10.0;

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

    nvgBeginFrame(vg, count, imageHeight, 1.0);
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
                0.0, mSpectrogramIm1, 1.0));
    nvgFill(vg);
   
    nvgRestore(vg);
    nvgSave(vg);

    double xp = (count - 1);

    if (!slice.empty()) {
        double frequency = slice[0].frequency;
        double intensity = slice[0].intensity;

        double y = frequencyToCoordinate(slice[0].frequency);

        double gain = intensity > 1e-10 ? 20.0 * log10f(intensity) : -1e6;
        double r, g, b;
        gainToColor(gain, &r, &g, &b);

        double yp = imageHeight - (y + 1.0) / 2.0 * imageHeight;

        for (const auto& point : slice) {
            double frequency = point.frequency;
            double intensity = point.intensity;

            double y2 = frequencyToCoordinate(frequency);

            double nextYp = imageHeight - (y2 + 1.0) / 2.0 * imageHeight;
           
            if (std::abs(std::round(yp) - std::round(nextYp)) >= 1) {
                double gain = intensity > 1e-10 ? 20.0 * log10f(intensity) : -1e6;
                double nr, ng, nb;
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
                nvgStrokeWidth(vg, 1.0);
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
                0.0, mSpectrogramIm1, 1.0));
    nvgFill(vg);
}

void NanoVG::renderFrequencyTrack(const FrequencyTrackRenderData& track, double thick, double r, double g, double b)
{
    nvgBeginPath(vg);
    nvgLineCap(vg, NVG_ROUND);
    nvgLineJoin(vg, NVG_ROUND);
    
    double xstep = (double) mWidth / (double) track.size();

    for (int i = 0; i < track.size(); ++i) {
        const auto& point = track[i];

        if (point.has_value()) {
            double xp = i * xstep;

            double y = frequencyToCoordinate(point.value());
            double yp = mHeight - (y + 1.0) / 2.0 * mHeight;

            if (i == 0 || !track[i - 1].has_value()) {
                nvgMoveTo(vg, xp, yp);
            }
            else {
                nvgLineTo(vg, xp, yp);
            }
        }
    }

    nvgStrokeWidth(vg, thick + 1.0);
    nvgStrokeColor(vg, nvgRGBAf(r, g, b, 1.0));
    nvgStroke(vg);

    nvgStrokeWidth(vg, 2.0);
    nvgStrokeColor(vg, nvgRGBAf(0, 0, 0, 1.0));
    nvgStroke(vg);
}

void NanoVG::renderFormantTrack(const FormantTrackRenderData& track, double thick, double r, double g, double b)
{
    double xstep = (double) mWidth / (double) track.size();

    for (int i = 0; i < track.size(); ++i) {
        const auto& point = track[i];

        if (point.has_value()) {
            const auto& formant = point.value();

            double xp = i * xstep;

            //double y1 = frequencyToCoordinate(formant.frequency - formant.bandwidth / 4.0);
            //double y1p = mHeight - (y1 + 1.0) / 2.0 * mHeight;

            //double y2 = frequencyToCoordinate(formant.frequency + formant.bandwidth / 4.0);
            //double y2p = mHeight - (y2 + 1.0) / 2.0 * mHeight;

            double y = frequencyToCoordinate(formant.frequency);
            double yp = mHeight - (y + 1.0) / 2.0 * mHeight;

            double cx = xp + xstep / 2.0;
            //double cy = (y1p + y2p) / 2.0;
            double cy = yp;
            double rx = xstep + 0.5;
            //double ry = std::max(fabsf(y1p - y2p) / 2.0, 4.0);
            double ry = 8.0;

            nvgBeginPath(vg);
            nvgEllipse(vg, cx, cy, rx, ry);
            nvgFillPaint(vg,
                    nvgRadialGradient(vg, cx, cy, ry / 4, ry,
                        nvgRGBf(r, g, b), nvgRGBAf(r, g, b, 1.0 / 1800.0)));
            nvgFill(vg);
        }
    }

    /*nvgBeginPath(vg);
    nvgLineCap(vg, NVG_ROUND);
    nvgLineJoin(vg, NVG_ROUND);

    double prevYp;

    for (int i = 0; i < track.size(); ++i) {
        const auto& point = track[i];

        if (point.has_value()) {
            const auto& formant = point.value();

            double xp = i * xstep;

            double y = frequencyToCoordinate(formant.frequency);
            double yp = mHeight - (y + 1.0) / 2.0 * mHeight;

            if (i == 0 || !track[i - 1].has_value()) {
                nvgMoveTo(vg, xp, yp);
            }
            else {
                double xmid = xp - xstep / 2.0;
                double ymid = (prevYp + yp) / 2.0;

                double cx1 = (xmid + xp - xstep) / 2;
                double cx2 = (xmid + xp + xstep) / 2;

                nvgQuadTo(vg, cx1, prevYp, xmid, ymid);
                nvgQuadTo(vg, cx2, yp, xp, yp);
            }

            prevYp = yp;
        }
    }

    nvgStrokeWidth(vg, thick + 1.0);
    nvgStrokeColor(vg, nvgRGBAf(1, 1, 1, 0.5));
    nvgStroke(vg);

    nvgStrokeWidth(vg, thick - 1.0);
    nvgStrokeColor(vg, nvgRGBAf(0, 0, 0, 1.0));
    nvgStroke(vg);*/

}

void NanoVG::renderFrequencyScaleBar(Module::Freetype::Font& majorFont, Module::Freetype::Font& minorFont)
{
    FrequencyScale scale = getParameters()->getFrequencyScale();
    double min = getParameters()->getMinFrequency();
    double max = getParameters()->getMaxFrequency();

    constexpr int    tickLens[2]   = {5, 3};
    constexpr double tickThicks[2] = {3, 2};

    if (scale == FrequencyScale::Linear) {

        int lo = 100 * (((int) min / 100) - 1);
        int hi = 100 * (((int) max / 100) + 1);

        for (int val = lo; val <= hi; val += 100) {
            if (val >= min && val < max) {
                double y = frequencyToCoordinate(val);
                double yp = mHeight - (y + 1.0) / 2.0 * mHeight;

                bool majorTick = (val % 1000 == 0);
                int tickLen                       = tickLens[!majorTick];
                double tickThick                   = tickThicks[!majorTick];
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
        double loLog = log10f(min);
        double hiLog = log10f(max);
        int loDecade = (int) floorf(loLog);

        double val;
        double startDecade = powf(10.0, loDecade);
            
        double decade = startDecade;
        double delta = hiLog - loLog, steps = fabs(delta);
        double step = delta >= 0 ? 10 : 0.1;
        double rMin = std::min(min, max), rMax = std::max(min, max);
        double start, end, sstep, mstep;
        
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
                    double y = frequencyToCoordinate(val);
                    double yp = mHeight - (y + 1.0) / 2.0 * mHeight;

                    bool majorTick = ((int) (f / 10.0) == f / 10.0);

                    int tickLen                       = tickLens[!majorTick];
                    double tickThick                   = tickThicks[!majorTick];
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

double NanoVG::renderFrequencyCursor(double mx, double my)
{
    double yp = my * mHeight;
    
    nvgBeginPath(vg);
    nvgMoveTo(vg, 0, yp);
    nvgLineTo(vg, mWidth, yp);
    nvgStrokeColor(vg, nvgRGBf(1, 1, 1));
    nvgStrokeWidth(vg, 3.0);
    nvgStroke(vg);
    
    double y = 2.0 * (0.5 - my);

    return coordinateToFrequency(y);
}

int NanoVG::renderFrameCursor(double mx, double my, int count)
{
    int frame = std::min<int>(std::max<int>(std::round(mx * count), 0), count - 1);
    double xp = (double) (mWidth * frame) / (double) count;

    nvgBeginPath(vg);
    nvgMoveTo(vg, xp, 0);
    nvgLineTo(vg, xp, mHeight);
    nvgStrokeColor(vg, nvgRGBf(1, 1, 1));
    nvgStrokeWidth(vg, 3.0);
    nvgStroke(vg);

    return frame;
}

void NanoVG::renderRoundedRect(double x, double y, double w, double h, double r, double g, double b, double a)
{
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x, y, w, h, 10.0);
    nvgFillColor(vg, nvgRGBAf(r, g, b, a));
    nvgFill(vg);
}

void NanoVG::renderSVG(const std::string& path, double dpi, double x, double y, double w, double h)
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
                vg, x, y, w, h, 0.0, image, 1.0));
    nvgFill(vg);
}

void NanoVG::renderText(Module::Freetype::Font& font, const std::string& text, int x0, int y0, double r, double g, double b)
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
                vg, x, y, glyphRenderData.width, glyphRenderData.height, 0.0, image, 1.0);

        paint.innerColor = nvgRGBAf(r, g, b, 1.0);

        nvgFillPaint(vg, paint);
        nvgFill(vg);
        
        x0 += glyphRenderData.advanceX >> 6;
    }
}

std::tuple<double, double, double, double> NanoVG::renderInputBox(Module::Freetype::Font& font, const std::string& content, int x, int y, int w, bool isFocused)
{
    int h = std::get<3>(font.queryTextSize("M")) + 10;
    
    NVGcolor startClr = isFocused ? nvgRGBA( 70, 102, 255, 128) : nvgRGBA(255, 255, 255,  32);
    NVGcolor endClr   = isFocused ? nvgRGBA( 32,  32,  32,  32) : nvgRGBA( 32,  32,  32,  32);

    NVGpaint bg;
    bg = nvgBoxGradient(vg, x+1,y+1+1.5, w-2,h-2, 3,4, startClr, endClr);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, x+1,y+1, w-2, h-2, 4-1);
    nvgFillPaint(vg, bg);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRoundedRect(vg, x+0.5,y+0.5, w-1,h-1, 4-0.5);
    nvgStrokeColor(vg, nvgRGBA(255,255,255,48));
    nvgStroke(vg);

    renderText(font, content, x + 5, y + 5, 1.0, 1.0, 1.0);

    return {
        (double) x,
        (double) y,
        (double) w,
        (double) h,
    };
}

std::pair<double, double> NanoVG::convertNormCoord(double x, double y)
{
    return {
        (x + 1.0) / 2.0 * mWidth,
        mHeight - (y + 1.0) / 2.0 * mHeight,
    };
}

uintptr_t NanoVG::getContextNumber()
{
    return (uintptr_t) (vg);
}
