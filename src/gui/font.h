#ifndef GUI_FONT_H
#define GUI_FONT_H

#include <rpcxx.h>
#include <QOpenGLFunctions>
#include <QSize>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Gui {

    struct FontCharacter {
        GLuint       texture;
        unsigned int width;
        unsigned int height;
        int          bearingX;
        int          bearingY;
        long         advance;
    };

    class Font {
    public:
        Font(FT_Library ft, const QString &font, int pixelSize);
        ~Font();
        
        const FontCharacter &charFor(char c);

    private:
        rpm::map<char, FontCharacter> mCharacters;
    };

}

#include "shaders/text.h"

#endif // GUI_FONT_H
