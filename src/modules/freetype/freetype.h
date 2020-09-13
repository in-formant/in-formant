#ifndef MODULES_FREETYPE_H
#define MODULES_FREETYPE_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <array>
#include <map>
#include <cstdint>
#include <climits>
#include <string>
#include <vector>
#include <optional>

namespace Module::Target {
    class AbstractBase;
}

namespace Module::Freetype {

    constexpr int maxTextLen = 128;

    void checkError(FT_Error error);

    struct GlyphRenderData {
        char character;
        unsigned int glyphIndex;
        int left, top; 
        int width, height;
        int advanceX, advanceY;
        std::vector<uint8_t> buffer;
        int deltaX; // only relevant when rendering.
    };

    struct TextRenderData {
        std::vector<GlyphRenderData> glyphs;
    };

    class FontFile;
    class Font;

    class FTInstance {
    public:
        FTInstance();
        ~FTInstance();
        
        FontFile& font(const std::string& filename);

    private:
        FT_Library mLibrary;

        std::map<std::string, FontFile *> mFontFiles;
    };

    class FontFile {
    public:
        FontFile(FT_Library library, const std::string& filename);
        ~FontFile();

        Font& with(int pointSize, Module::Target::AbstractBase *target);

    private:
        FT_Library mLibrary;
        
        FT_Byte *mData;
        int mDataSize;

        std::map<std::tuple<int, int, int>, Font *> mFonts;
    };

    class Font {
    public:
        Font(FT_Library library,
                const FT_Byte *fileData, int fileDataSize,
                int pointSize, int hdpi, int vdpi);
        ~Font();

        GlyphRenderData prepareCharRender(char character);
        TextRenderData prepareTextRender(const std::string& text);

        void queryBBox(const std::string& text, FT_BBox *abbox);

        void setAttachment(void *p) {
            mAttachment = p;
        }

        template<typename T>
        T *getAttachment() {
            return static_cast<T *>(mAttachment);
        }

        bool hasAttachment() const {
            return mAttachment != nullptr;
        }

    private:
        FT_Face mFace;
        
        std::array<GlyphRenderData, UCHAR_MAX> mGlyphsData;

        void *mAttachment;
    };

}

#endif // MODULES_FREETYPE_H
