#ifndef GUI_CANVAS_RENDERER_H
#define GUI_CANVAS_RENDERER_H

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QQuickFramebufferObject>
#include <QColor>
#include <string>

#include "font.h"
#include "shaders/spec.h"
#include "../context/datastore.h"

namespace Main {
    class RenderContext;
}

namespace Gui {

    class CanvasRenderer : protected QOpenGLExtraFunctions {
    public:
        void initialize(Main::RenderContext *renderContext);
        void cleanup();
        void synchronize(QQuickFramebufferObject *item);
        
        void render();

        void setZoomScale(double zoomScale);

        void drawTextNormal(float x, float y, const QColor &color, const std::string &text);
        void drawTextNormalOutlined(float x, float y, const QColor &color, const std::string &text, const QColor &outlineColor = Qt::black);
        QRect textBoundsNormal(const std::string &text);

        void drawTextSmall(float x, float y, const QColor &color, const std::string &text);
        void drawTextSmallOutlined(float x, float y, const QColor &color, const std::string &text, const QColor &outlineColor = Qt::black);
        QRect textBoundsSmall(const std::string &text);

        void drawTextSmaller(float x, float y, const QColor &color, const std::string &text);
        void drawTextSmallerOutlined(float x, float y, const QColor &color, const std::string &text, const QColor &outlineColor = Qt::black);
        QRect textBoundsSmaller(const std::string &text);

        void drawScatterWithOutline(const rpm::vector<QPointF> &points, float radius, const QColor &fillColor, const QColor &outlineColor = Qt::black);
    
        void drawPoint(const QPointF &point, float radius, const QColor &color);

        void drawLine(float x1, float y1, float x2, float y2, const QColor &color, float thickness);

        void prepareSpectrogramDraw();
        void drawSpectrogram(
                int xOffset,
                int chunkSize1,
                int chunkSize2,
                int totalSize,
                const rpm::vector<GLint>& nffts,
                const rpm::vector<GLfloat>& sampleRates,
                const rpm::vector<GLfloat>& chunkData1,
                const rpm::vector<GLfloat>& chunkData2,
                FrequencyScale freqScale,
                float minFrequency,
                float maxFrequency,
                float maxGain,
                const QVector<QRgb>& colorTable,
                float sliceTimeStart,
                float sliceTimeEnd,
                float timeStart,
                float timeEnd);

        QRect viewport() const;

    private:
        void initFonts();
        void initShaders();
        void deleteFonts();
        void deleteShaders();

        void initTexture(GLuint &texture, int width, int height);
        QOpenGLShaderProgram *createShaderProgram(const char *vertexSource, const char *fragmentSource);

        void drawText(Font *font, float x, float y, const QColor &color, const std::string &text);
        void drawTextOutlined(Font *font, float x, float y, const QColor &color, const std::string &text, const QColor &outlineColor = Qt::black);
        QRect textBounds(Font *font, const std::string &text);

        Main::RenderContext *mRenderContext;

        double mDevicePixelRatio;
        int mWidth, mHeight;
        double mDpi;
        double mZoomScale, mZoomScaleText;

        Font *mFontNormal;
        Font *mFontSmall;
        Font *mFontSmaller;

        QOpenGLShaderProgram *mTextProgram;
        GLuint mTextVao, mTextVbo;

        QOpenGLShaderProgram *mSpecProgram;
        GLuint mSpecVao, mSpecVbo;

        QOpenGLShaderProgram *mSpec2Program;
        GLuint mSpec2Vao, mSpec2Vbo;

        std::optional<GLuint> mDefaultFbo;
        GLuint mSpecTex;

        int mSpecTexMaxWidth, mSpecTexMaxHeight;
    };

}

#endif // GUI_CANVAS_RENDERER_H
