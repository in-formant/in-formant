#ifndef MODULES_FREETYPE_H
#define MODULES_FREETYPE_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include <cstdint>
#include <climits>
#include <string>
#include <vector>
#include <optional>

namespace Module::Freetype {

    constexpr int maxTextLen = 128;

    void checkError(FT_Error error);

    class FTInstance {
    public:
        FTInstance();
        ~FTInstance();
    
        operator FT_Library();

        const std::vector<FT_Byte>& getFileData(const std::string& filename);

    private:
        FT_Library mLibrary;

        std::map<std::string, std::vector<FT_Byte>> mFilesData;
    };
    
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

    class Font {
    public:
        Font(FTInstance& library, const std::string& filename, int pixelSize);
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
        int mSize;

        std::array<GlyphRenderData, UCHAR_MAX> mGlyphsData;

        void *mAttachment;
    };

}

#endif // MODULES_FREETYPE_H
