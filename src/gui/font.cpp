#include "font.h"
#include <QFile>
#include <iostream>

using namespace Gui;

Font::Font(FT_Library ft, const QString &font, int pixelSize)
{
    QFile fontFile(font);
    if (!fontFile.open(QIODevice::ReadOnly)) {
        std::cout << "ERROR::FREETYPE: Failed to read font file" << std::endl;
        return;
    }
    QByteArray buffer = fontFile.readAll();
    fontFile.close();

    FT_Face face;
    if (FT_New_Memory_Face(ft, (const FT_Byte *) buffer.constData(), buffer.size(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, pixelSize);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 255; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYPE: Failed to load glyph" << std::endl;
            continue;
        }
        
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        mCharacters.emplace((char) c, FontCharacter {
            texture,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            face->glyph->bitmap_left,
            face->glyph->bitmap_top,
            face->glyph->advance.x 
        });
    }

    FT_Done_Face(face);
}

Font::~Font()
{
    for (const auto& [c, character] : mCharacters) {
        glDeleteTextures(1, &character.texture);
    }
}

const FontCharacter &Font::charFor(char c)
{
    return mCharacters[c];
}
