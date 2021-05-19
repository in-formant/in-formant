#include "canvas.h"
#include "../context/timings.h"
#include "../context/rendercontext.h"
#include "qpainterwrapper.h"
#include <iostream>
#include <cmath>

using namespace Gui;

constexpr GLuint maxTextureWidth = 4096;
constexpr GLuint maxTextureHeight = 16384;

void CanvasRenderer::initialize(Main::RenderContext *renderContext)
{
    mRenderContext = renderContext;
    initializeOpenGLFunctions();
    initFonts();
    initShaders();

    mSpecTex.resize(4);
    for (auto& tex : mSpecTex) {
        initTexture(tex);
    }
}

void CanvasRenderer::cleanup()
{
    deleteFonts();
    deleteShaders();

    glDeleteTextures(mSpecTex.size(), mSpecTex.data());
}

void CanvasRenderer::synchronize(QQuickFramebufferObject *item)
{
    mWidth = item->width();
    mHeight = item->height();
}

void CanvasRenderer::render()
{
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
  
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    glOrtho(0, mWidth, mHeight, 0, -1, 1);
    glViewport(0, 0, mWidth, mHeight);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    QPainterWrapper painter(this);
    mRenderContext->render(&painter);
    
    std::stringstream ss1;
    ss1 << "Render: " << timings::render;
    auto renderTimeString = ss1.str();

    std::stringstream ss2;
    ss2 << "Update: " << timings::update;
    auto updateTimeString = ss2.str();

    /*std::cout << "Update spectrogram:  " << timings::updateSpectrogram << "\n"
              << "       pitch:        " << timings::updatePitch << "\n"
              << "       formants:     " << timings::updateFormants << "\n"
              << "       oscilloscope: " << timings::updateOscilloscope << "\n\n";*/

    float y = 10;
    QRect textBox;

    textBox = textBoundsNormal(renderTimeString);
    y += textBox.height();
    drawTextNormalOutlined(10, y, Qt::white, renderTimeString);

    textBox = textBoundsNormal(updateTimeString);
    y += textBox.height() + 10;
    drawTextNormalOutlined(10, y, Qt::white, updateTimeString);
}

void CanvasRenderer::drawTextNormal(float x, float y, const QColor &color, const std::string &text)
{
    drawText(mFontNormal, x, y, color, text);
}

void CanvasRenderer::drawTextNormalOutlined(float x, float y, const QColor &color, const std::string &text, const QColor &outlineColor)
{
    drawTextOutlined(mFontNormal, x, y, color, text, outlineColor);
}

QRect CanvasRenderer::textBoundsNormal(const std::string &text)
{
    return textBounds(mFontNormal, text);
}

void CanvasRenderer::drawTextSmall(float x, float y, const QColor &color, const std::string &text)
{
    drawText(mFontSmall, x, y, color, text);
}

void CanvasRenderer::drawTextSmallOutlined(float x, float y, const QColor &color, const std::string &text, const QColor &outlineColor)
{
    drawTextOutlined(mFontSmall, x, y, color, text, outlineColor);
}

QRect CanvasRenderer::textBoundsSmall(const std::string &text)
{
    return textBounds(mFontSmall, text);
}

void CanvasRenderer::drawTextSmaller(float x, float y, const QColor &color, const std::string &text)
{
    drawText(mFontSmaller, x, y, color, text);
}

void CanvasRenderer::drawTextSmallerOutlined(float x, float y, const QColor &color, const std::string &text, const QColor &outlineColor)
{
    drawTextOutlined(mFontSmaller, x, y, color, text, outlineColor);
}

QRect CanvasRenderer::textBoundsSmaller(const std::string &text)
{
    return textBounds(mFontSmaller, text);
}

void CanvasRenderer::drawScatterWithOutline(const rpm::vector<QPointF> &points, float radius, const QColor &fillColor, const QColor &outlineColor)
{
    for (const auto& point : points) {
        drawPoint(point, radius + 0.6667, outlineColor);
    }
    for (const auto& point : points) {
        drawPoint(point, radius, fillColor);
    }
}

void CanvasRenderer::drawPoint(const QPointF &point, float radius, const QColor &color)
{
    constexpr int N = 30;
    float vertices[2*(N+1)];
    for (int i = 0; i <= N; ++i) {
        float angle = 2 * M_PI * i / N;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        vertices[2*i] = point.x() + x;
        vertices[2*i+1] = mHeight - point.y() + y;
    }
    glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, N+1);
}

void CanvasRenderer::drawLine(float x1, float y1, float x2, float y2, const QColor &color, float thickness)
{
    y1 = mHeight - y1;
    y2 = mHeight - y2;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float d = sqrt(dx * dx + dy * dy);

    float nx = -dy / d;
    float ny = dx / d;

    float qx1 = x1 - 0.5 * thickness * nx;
    float qx2 = x2 + 0.5 * thickness * nx;
    float qy1 = y1 - 0.5 * thickness * ny;
    float qy2 = y2 + 0.5 * thickness * ny;

    glColor4f(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(qx1, qy1);
    glVertex2f(qx1, qy2);
    glVertex2f(qx2, qy2);
    glVertex2f(qx2, qy1);
    glEnd();
}

void CanvasRenderer::prepareSpectrogramDraw()
{
    std::rotate(mSpecTex.begin(), mSpecTex.begin() + 1, mSpecTex.end());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mSpecTex[0]);
}

void CanvasRenderer::drawSpectrogram(
        float fftFrequency,
        FrequencyScale freqScale,
        float minFrequency,
        float maxFrequency,
        float maxGain,
        const QVector<QRgb>& colorTable,
        float x1,
        float x2,
        float y1,
        float y2)
{   
    mSpecProgram->bind();

    QMatrix4x4 projection;
    projection.ortho(0, mWidth, mHeight, 0, -1, 1);
    mSpecProgram->setUniformValue("projection", projection);

    std::array<QVector3D, 256> cmap;
    for (int i = 0; i < 256; ++i) {
        QColor color = QColor::fromRgb(colorTable[i]);
        cmap[i] = QVector3D(color.redF(), color.greenF(), color.blueF());
    }
    mSpecProgram->setUniformValueArray("colorMap", cmap.data(), 256);

    mSpecProgram->setUniformValue("fftFrequency", fftFrequency);
    mSpecProgram->setUniformValue("frequencyScale", static_cast<int>(freqScale));
    mSpecProgram->setUniformValue("minFrequency", minFrequency);
    mSpecProgram->setUniformValue("maxFrequency", maxFrequency);
    mSpecProgram->setUniformValue("maxGain", maxGain);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(mSpecVao);

    float vertices[4][4] = {
        { x2, y1, 1.0f, 1.0f },
        { x2, y2, 1.0f, 0.0f },
        { x1, y2, 0.0f, 0.0f },
        { x1, y1, 0.0f, 1.0f },
    };
    
    glBindTexture(GL_TEXTURE_2D, mSpecTex[0]);

    glBindBuffer(GL_ARRAY_BUFFER, mSpecVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    mSpecProgram->release();
}

QRect CanvasRenderer::viewport() const
{
    return QRect(0, 0, mWidth, mHeight);
}

void CanvasRenderer::drawText(Font *font, float x, float y, const QColor &color, const std::string &text)
{
    mTextProgram->bind();

    y = mHeight - y;

    QMatrix4x4 projection;
    projection.ortho(0, mWidth, mHeight, 0, -1, 1);
    mTextProgram->setUniformValue("projection", projection);
    mTextProgram->setUniformValue("textColor", color.redF(), color.greenF(), color.blueF());

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(mTextVao);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); ++c) {
        if (*c == '\0') break;

        FontCharacter ch = font->charFor(*c);

        float xpos = x + ch.bearingX;
        float ypos = y - (ch.height - ch.bearingY);

        float w = ch.width;
        float h = ch.height;

        float vertices[4][4] = {
            { xpos + w, ypos,     1.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 0.0f },
            { xpos,     ypos + h, 0.0f, 0.0f },
            { xpos,     ypos,     0.0f, 1.0f },
        };

        glBindTexture(GL_TEXTURE_2D, ch.texture);
        
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, mTextVbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        x += (ch.advance >> 6);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    mTextProgram->release();
}

void CanvasRenderer::drawTextOutlined(Font *font, float x, float y, const QColor &color, const std::string &text, const QColor &outlineColor)
{
    constexpr float delta = 0.66667;

    for (int dxi = -1; dxi <= 1; dxi += 2) {
        for (int dyi = -1; dyi <= 1; dyi += 2) {
            drawText(font, x + delta * dxi, y + delta * dyi, outlineColor, text);
        }
    }
    drawText(font, x, y, color, text);
}

QRect CanvasRenderer::textBounds(Font *font, const std::string &text)
{
    float x = 0;
    float y = 0;
    
    QRect rect;
    float xmax = 0;
    float ymax = 0;

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); ++c) {
        FontCharacter ch = font->charFor(*c);

        float xpos = x + ch.bearingX;
        float ypos = y - (ch.height - ch.bearingY);

        float w = ch.width;
        float h = ch.height;

        if (c == text.begin()) {
            rect.setX(xpos);
            rect.setY(ypos);
        }

        if (xpos + w > xmax) {
            xmax = xpos + w;
        }
        if (ypos + h > ymax) {
            ymax = ypos + h;
        }

        x += (ch.advance >> 6);
    }

    rect.setWidth(xmax - rect.x());
    rect.setHeight(ymax - rect.y());

    return rect;
}

void CanvasRenderer::initFonts()
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType library" << std::endl;
        return;
    }

    mFontNormal = new Font(ft, ":/Roboto.ttf", 18);
    mFontSmall = new Font(ft, ":/Roboto.ttf", 16);
    mFontSmaller = new Font(ft, ":/Roboto.ttf", 14);

    FT_Done_FreeType(ft);
}

void CanvasRenderer::initShaders()
{
    mTextProgram = createShaderProgram(Shaders::textVertex, Shaders::textFragment);

    glGenVertexArrays(1, &mTextVao);
    glGenBuffers(1, &mTextVbo);
    glBindVertexArray(mTextVao);
    glBindBuffer(GL_ARRAY_BUFFER, mTextVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    mSpecProgram = createShaderProgram(Shaders::specVertex, Shaders::specFragment);

    glGenVertexArrays(1, &mSpecVao);
    glGenBuffers(1, &mSpecVbo);
    glBindVertexArray(mSpecVao);
    glBindBuffer(GL_ARRAY_BUFFER, mSpecVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void CanvasRenderer::deleteFonts()
{
    delete mFontNormal;
    delete mFontSmall;
    delete mFontSmaller;
}

void CanvasRenderer::deleteShaders()
{
    delete mTextProgram;
    glDeleteVertexArrays(1, &mTextVao);
    glDeleteBuffers(1, &mTextVbo);

    delete mSpecProgram; 
    glDeleteVertexArrays(1, &mSpecVao);
    glDeleteBuffers(1, &mSpecVbo);
}

void CanvasRenderer::initTexture(GLuint &texture)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxTextureWidth, maxTextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

QOpenGLShaderProgram *CanvasRenderer::createShaderProgram(
        const char *vertexSource, const char *fragmentSource)
{
    auto program = new QOpenGLShaderProgram();
    program->addCacheableShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource);
    program->addCacheableShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource);
    program->link();
    return program;
}

