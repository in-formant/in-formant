#include "opengl.h"

using namespace Module::Renderer;

OGL::FontAttachment::FontAttachment(Module::Freetype::Font& font)
{
    glCreateTextures(GL_TEXTURE_2D, UCHAR_MAX, mGlyphsTex.data());

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char uch = 0; uch < UCHAR_MAX; ++uch) {
        auto glyphData = font.prepareCharRender((char) uch);

        glBindTexture(GL_TEXTURE_2D, mGlyphsTex[uch]);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     glyphData.width,
                     glyphData.height,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     glyphData.buffer.data());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

   glBindTexture(GL_TEXTURE_2D, 0); 
}

GLuint OGL::FontAttachment::getTextureFor(char ch)
{
    return mGlyphsTex[(unsigned char) ch];
}
