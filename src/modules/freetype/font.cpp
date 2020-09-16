#include "freetype.h"
#include <stdexcept>
#include <iostream>
#include <cmath>
#include FT_TRUETYPE_IDS_H

using namespace Module::Freetype;

Font::Font(FT_Library library,
            const FT_Byte *fileData,
            int fileDataSize,
            int pointSize, int hdpi, int vdpi)
    : mAttachment(nullptr)
{
    FT_Error err;

    FT_Open_Args openArgs {
        .flags = FT_OPEN_MEMORY,
        .memory_base = fileData,
        .memory_size = fileDataSize,
    };

    err = FT_Open_Face(library, &openArgs, 0, &mFace);
    checkError(err);

    err = FT_Select_Charmap(mFace, FT_ENCODING_UNICODE);
    checkError(err);

    err = FT_Set_Char_Size(mFace, 0, pointSize * 64, hdpi, vdpi);
    checkError(err);

    for (unsigned char uch = 0; uch < UCHAR_MAX; ++uch) {
        char character = (char) uch;

        FT_UInt glyphIndex = FT_Get_Char_Index(mFace, character);
        if (glyphIndex == 0) {
            mGlyphsData[uch] = GlyphRenderData {
                .character = character,
                .glyphIndex = 0,
                .left = 0,
                .top = 0,
                .width = 0,
                .height = 0,
                .advanceX = 0,
                .advanceY = 0,
                .buffer = {},
            }; 
            continue;
        }

        err = FT_Load_Glyph(mFace, glyphIndex, FT_LOAD_DEFAULT);
        checkError(err);

        err = FT_Render_Glyph(mFace->glyph, FT_RENDER_MODE_NORMAL);
        checkError(err);

        int rows = mFace->glyph->bitmap.rows;
        int width = mFace->glyph->bitmap.width;

        std::vector<uint8_t> buffer(rows * width);

        for (int row = 0; row < rows; ++row) {
            for (int x = 0; x < width; ++x) {
                int i = row * width + x;
                buffer[i] = mFace->glyph->bitmap.buffer[i];
            }
        }

        mGlyphsData[uch] = GlyphRenderData {
            .character = character,
            .glyphIndex = mFace->glyph->glyph_index, 
            .left = mFace->glyph->bitmap_left,
            .top = mFace->glyph->bitmap_top,
            .width = width,
            .height = rows,
            .advanceX = (int) mFace->glyph->advance.x,
            .advanceY = (int) mFace->glyph->advance.y,
            .buffer = std::move(buffer),
        };
    }
}

Font::~Font()
{
    FT_Done_Face(mFace);
}

GlyphRenderData Font::prepareCharRender(char character)
{
    return mGlyphsData[(unsigned char) character];
}

TextRenderData Font::prepareTextRender(const std::string& text)
{
    std::vector<GlyphRenderData> glyphs(text.size());
    
    bool useKerning = FT_HAS_KERNING(mFace);
    unsigned int previousGlyphIndex = 0;

    for (int i = 0; i < text.size(); ++i) {
        GlyphRenderData glyphData = mGlyphsData[(unsigned char) text[i]];
        unsigned int glyphIndex = glyphData.glyphIndex;

        if (useKerning && previousGlyphIndex && glyphIndex) {
            FT_Vector delta;
            FT_Get_Kerning(mFace, previousGlyphIndex, glyphIndex, FT_KERNING_DEFAULT, &delta);
            glyphData.left += delta.x >> 6;
        }

        glyphs[i] = std::move(glyphData);

        previousGlyphIndex = glyphIndex;
    }

    TextRenderData data = {
        .glyphs = std::move(glyphs),
    };
    
    return std::move(data);
}

std::array<float, 4> Font::queryTextSize(const std::string& text)
{
    TextRenderData textRenderData = prepareTextRender(text);

    float xmin(HUGE_VALF), xmax(-HUGE_VALF);
    float ymin(HUGE_VALF), ymax(-HUGE_VALF);

    int x0 = 0;
    int y0 = 0;

    for (const auto& glyphRenderData : textRenderData.glyphs) {
        float x = x0 + glyphRenderData.left;
        float y = y0 - glyphRenderData.top;

        float w = glyphRenderData.width;
        float h = glyphRenderData.height;

        x0 += glyphRenderData.advanceX >> 6;

        xmin = std::min(x, xmin);
        ymin = std::min(y, ymin);

        xmax = std::max(x + w, xmax);
        ymax = std::max(y + h, ymax);
    }
    
    float textWidth = xmax - xmin;
    float textHeight = ymax - ymin;
    
    return {xmin, ymin, textWidth, textHeight};
}

