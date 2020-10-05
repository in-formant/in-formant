#ifndef RENDERER_NVG_H
#define RENDERER_NVG_H

#include "../base/base.h"

namespace Module::Renderer {

    namespace NvgUtils {

        class FontAttachment {
        public:
            FontAttachment(NVGcontext *vg, Module::Freetype::Font& font);
            ~FontAttachment();

            int getImageFor(char character) const;

        private:
            NVGcontext *vg;
            std::array<int, UCHAR_MAX> mGlyphsImages;

            struct pixel_data {
                uint8_t r:8;
                uint8_t g:8;
                uint8_t b:8;
                uint8_t a:8;
            };

            std::array<std::vector<pixel_data>, UCHAR_MAX> mGlyphsData;
        };

    }

    class NanoVG : public AbstractBase {
    public:
        NanoVG();
        ~NanoVG();

        void setProvider(void *provider) override;

        void initialize() override;
        void terminate() override;

        void begin() override;
        void end() override;

        void test() override;
 
        void renderGraph(const GraphRenderData& data, float pmin, float pmax, float thick, float r, float g, float b) override;
        void scrollSpectrogram(const SpectrogramRenderData& data, int count) override;
        void renderSpectrogram() override;
        void renderFrequencyTrack(const FrequencyTrackRenderData& data, float thick, float r, float g, float b) override;
        void renderFormantTrack(const FormantTrackRenderData& data, float r, float g, float b) override;
        void renderFrequencyScaleBar(Module::Freetype::Font& majorFont, Module::Freetype::Font& minorFont) override;
        float renderFrequencyCursor(float mx, float my) override;
        int renderFrameCursor(float mx, float my, int count) override;
        
        void renderRoundedRect(float x, float y, float w, float h, float r, float g, float b, float a) override;
        void renderSVG(const std::string& path, float dpi, float x, float y, float w, float h) override;

        void renderText(Module::Freetype::Font& font, const std::string& text, int x, int y, float r, float g, float b) override;

        std::tuple<float, float, float, float> renderInputBox(Module::Freetype::Font& font, const std::string& content, int x, int y, int w, bool isFocused) override;

        uintptr_t getContextNumber() override;

    private:
        std::pair<float, float> convertNormCoord(float x, float y);

        NvgProvider *mProvider;
        
        NVGcontext *vg;

        int mWidth, mHeight;
       
        int mSpectrogramCount; 
        void *mSpectrogramFb1, *mSpectrogramFb2;
        int mSpectrogramIm1, mSpectrogramIm2;

        std::map<std::string, int> mSvgImages;
        std::map<std::string, std::vector<uint8_t>> mSvgImageData;
    };

}

#endif
