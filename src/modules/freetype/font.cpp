#include "freetype.h"
#include <stdexcept>
#include <iostream>
#include FT_TRUETYPE_IDS_H

using namespace Module::Freetype;

Font::Font(FTInstance& library, const std::string& filename, int pixelSize)
    : mSize(pixelSize),
      mAttachment(nullptr)
{
    const auto& fileData = library.getFileData(filename);

    FT_Error err;

    err = FT_New_Memory_Face(library, fileData.data(), fileData.size(), 0, &mFace);
    checkError(err);

    err = FT_Select_Charmap(mFace, FT_ENCODING_UNICODE);
    checkError(err);

    err = FT_Set_Pixel_Sizes(mFace, 0, pixelSize);
    checkError(err);

    for (unsigned char uch = 0; uch < UCHAR_MAX; ++uch) {
        mGlyphsData[uch] = prepareCharRender((char) uch);
    }
}

Font::~Font()
{
    FT_Done_Face(mFace);
}

GlyphRenderData Font::prepareCharRender(char character)
{
    unsigned char uch = (unsigned char) character;

    FT_UInt glyphIndex = FT_Get_Char_Index(mFace, character);
    if (glyphIndex == 0) {
        return GlyphRenderData {
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
    }
    else {
        FT_Error err;

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

        return GlyphRenderData {
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

void Font::queryBBox(const std::string& text, FT_BBox *abbox)
{
    FT_BBox bbox;

    bbox.xMin = bbox.yMin =  32000;
    bbox.xMax = bbox.yMax = -32000;

    for (int i = 0; i < text.size(); ++i) {

    }
}
