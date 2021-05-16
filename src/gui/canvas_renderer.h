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

        void prepareSpectrogramDraw(const QVector<QRgb>& colorTable);
        void drawSpectrogram(
                float fftFrequency,
                FrequencyScale freqScale,
                float minFrequency,
                float maxFrequency,
                float x1, float x2);

        QRect viewport() const;

    private:
        void initFonts();
        void initFramebuffers();
        //void initTextures();
        void initShaders();
        void deleteFonts();
        void deleteFramebuffers();
        void deleteShaders();

        void initFramebuffer(GLuint &fbo, GLuint &texture);
        void initTexture(GLuint &texture);
        void deleteFramebuffer(GLuint fbo, GLuint texture);
        QOpenGLShaderProgram *createShaderProgram(const char *vertexSource, const char *fragmentSource);

        void drawText(Font *font, float x, float y, const QColor &color, const std::string &text);
        void drawTextOutlined(Font *font, float x, float y, const QColor &color, const std::string &text, const QColor &outlineColor = Qt::black);
        QRect textBounds(Font *font, const std::string &text);

        Main::RenderContext *mRenderContext;

        int mWidth, mHeight;

        Font *mFontNormal;
        Font *mFontSmall;
        Font *mFontSmaller;

        GLuint mDefaultFbo;

        QOpenGLShaderProgram *mTextProgram;
        GLuint mTextVao, mTextVbo;

        QOpenGLShaderProgram *mSpecProgram;
        GLuint mSpecVao, mSpecVbo;
        GLuint mSpecFbo, mSpecTex;
    };

}

#endif // GUI_CANVAS_RENDERER_H
