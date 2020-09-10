#include "nanovg.h"

using namespace Module::Renderer;

NvgUtils::FontAttachment::FontAttachment(NVGcontext *vg, Module::Freetype::Font& font)
    : vg(vg)
{
    for (unsigned char uch = 0; uch < UCHAR_MAX; ++uch) {
        auto glyphData = font.prepareCharRender((char) uch);

        int width = glyphData.width;
        int height = glyphData.height;

        auto& pixels = mGlyphsData[uch];

        pixels.resize(width * height);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint8_t a = glyphData.buffer[y * width + x];

                pixels[y * width + x] = {
                   .r = 255,
                   .g = 255,
                   .b = 255,
                   .a = a,
                };
            }
        }

        mGlyphsImages[uch] = nvgCreateImageRGBA(vg, width, height, 0, reinterpret_cast<uint8_t *>(pixels.data()));
    }
}

NvgUtils::FontAttachment::~FontAttachment()
{
    for (unsigned char uch = 0; uch < UCHAR_MAX; ++uch) {
        nvgDeleteImage(vg, mGlyphsImages[uch]);
    }
}

int NvgUtils::FontAttachment::getImageFor(char character) const
{
    return mGlyphsImages[(unsigned char) character];
}
