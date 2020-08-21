#include "opengl.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace Module::Renderer;

OpenGL::OpenGL()
    : AbstractBase(Type::OpenGL),
      mProvider(nullptr)
{
    mMsCount = 4;
}

OpenGL::~OpenGL()
{
}

void OpenGL::setProvider(void *provider)
{
    mProvider = static_cast<OpenGLProvider *>(provider);
}

void OpenGL::initialize()
{
    mProvider->createContext();

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        throw std::runtime_error(std::string("Renderer::OpenGL] Error initializing GLEW: ") + (const char *) glewGetErrorString(err));
    }

    if (!GLEW_VERSION_3_2) {
        throw std::runtime_error(std::string("Renderer::OpenGL] Required OpenGL 3.2 or higher could not be found"));
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGL::debugCallback, nullptr);

    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);

    // Create FBO and RBO.
    glGenFramebuffers(1, &mFBO);
    glGenRenderbuffers(1, &mRBO);
    
    // Init multisampling.
    glGenTextures(1, &mMsTexture);

    // Create VAO.
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    // Create VBO.
    glGenBuffers(1, &mVBO);

    // Create shader programs.
    mGraphProgram = new OGL::ShaderProgram("graph");
    mSpectrogramProgram = new OGL::ShaderProgram("spectrogram");
    mTextProgram = new OGL::ShaderProgram("text");

    // Init and map VBO buffer.
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, 32768, nullptr, GL_DYNAMIC_DRAW);

    onDrawableSizeChanged();
}

void OpenGL::terminate()
{
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
    glDeleteFramebuffers(1, &mFBO);
    glDeleteRenderbuffers(1, &mRBO);
    glDeleteTextures(1, &mMsTexture);
    delete mGraphProgram;
    delete mSpectrogramProgram;
    delete mTextProgram;
    
    mProvider->deleteContext();
}

void OpenGL::begin()
{
    mProvider->makeCurrent();

    if (hasDrawableSizeChanged()) {
        onDrawableSizeChanged();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void OpenGL::end()
{
    int width, height;
    getDrawableSize(&width, &height);
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    mProvider->swapTarget();
}

void OpenGL::clear()
{
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    GLfloat color0[4] { 0.0f, 0.0f, 0.0f, 1.0f };
    glClearBufferfv(GL_COLOR, 0, (GLfloat *) color0);
}

void OpenGL::test()
{
    mGraphProgram->use();

    glUniform1f(mGraphProgram->unif("offset_x"), 0.0f);
    glUniform1f(mGraphProgram->unif("scale_x"), 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);

    glEnableVertexAttribArray(mGraphProgram->attr("coord2d"));
    glVertexAttribPointer(
        mGraphProgram->attr("coord2d"),
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void *) 0
    );

    glDrawArrays(GL_LINE_STRIP, 0, 2000);

    glDisableVertexAttribArray(mGraphProgram->attr("coord2d"));
}

void OpenGL::renderGraph(float *x, float *y, size_t count)
{
    mGraphProgram->use();

    glUniform1f(mGraphProgram->unif("offset_x"), 0.0f);
    glUniform1f(mGraphProgram->unif("scale_x"), 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    
    auto graph = (Vertex *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    for (int i = 0; i < count; ++i) {
        graph[i].x = 0.9f * (2.0f * (x[i] - x[0]) / (x[count - 1] - x[0]) - 1.0f);
        graph[i].y = y[i] / 10.0f;
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glEnableVertexAttribArray(mGraphProgram->attr("coord2d"));
    glVertexAttribPointer(
        mGraphProgram->attr("coord2d"),
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void *) 0
    );

    glDrawArrays(GL_LINE_STRIP, 0, count);

    glDisableVertexAttribArray(mGraphProgram->attr("coord2d")); 
}

void OpenGL::renderSpectrogram(float ***spectrogram, size_t *lengths, size_t count)
{
    mSpectrogramProgram->use();

    auto par = getParameters();
    glUniform1f(mSpectrogramProgram->unif("minFreq"), par->getMinFrequency());
    glUniform1f(mSpectrogramProgram->unif("maxFreq"), par->getMaxFrequency());
    glUniform1f(mSpectrogramProgram->unif("minGain"), par->getMinGain());
    glUniform1f(mSpectrogramProgram->unif("maxGain"), par->getMaxGain());
    glUniform1ui(mSpectrogramProgram->unif("scaleType"), (GLuint) par->getFrequencyScale());

    int tWidth, tHeight;
    getDrawableSize(&tWidth, &tHeight);

    glLineWidth((float) tWidth / (float) count);

    std::vector<glm::vec3> vertices;

    for (int xi = 0; xi < count; ++xi) {
        float **slice = spectrogram[xi];
        size_t length = lengths[xi];

        float x = (2.0f * (xi + 0.5f)) / (float) count - 1.0f;

        vertices.resize(length);
       
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);

        for (int yi = 0; yi < length; ++yi) {
            float frequency = slice[yi][0];
            float intensity = slice[yi][1];

            float gain = intensity > 1e-10f ? 20.0f * log10(intensity) : -1e6f;

            vertices[yi] = { x, frequency, gain };
        }
       
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), vertices.data()); 

        glEnableVertexAttribArray(mSpectrogramProgram->attr("point"));
        glVertexAttribPointer(
            mSpectrogramProgram->attr("point"),
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *) 0
        );

        glDrawArrays(GL_LINE_STRIP, 0, vertices.size());

        glDisableVertexAttribArray(mSpectrogramProgram->attr("point"));
    }
}

void OpenGL::renderFrequencyTrack(float *track, size_t count)
{
    mSpectrogramProgram->use();

    auto par = getParameters();
    glUniform1f(mSpectrogramProgram->unif("minFreq"), par->getMinFrequency());
    glUniform1f(mSpectrogramProgram->unif("maxFreq"), par->getMaxFrequency());
    glUniform1ui(mSpectrogramProgram->unif("scaleType"), (GLuint) par->getFrequencyScale());

    float width = 2.0f / (float) count;

    int tWidth, tHeight;
    getDrawableSize(&tWidth, &tHeight);

    glLineWidth(tWidth / (float) count);

    std::vector<glm::vec3> vertices;
    vertices.reserve(count);

    int xi = 0;

    while (xi < count) {
        if (track[xi] < 0) {
            if (xi > 0 && track[xi - 1] > 0) {
                glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(decltype(vertices)::value_type), vertices.data()); 
                glEnableVertexAttribArray(mSpectrogramProgram->attr("point"));
                glVertexAttribPointer(
                    mSpectrogramProgram->attr("point"),
                    3,
                    GL_FLOAT,
                    GL_FALSE,
                    0,
                    (void *) 0
                );
                glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
                glDisableVertexAttribArray(mSpectrogramProgram->attr("point"));
                vertices.clear();
            }
        }
        else {
            float x = (2.0f * xi) / (float) count - 1.0f;
            vertices.push_back({ x, track[xi], -20 });
        }
        xi++;
    }

    if (!vertices.empty()) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(decltype(vertices)::value_type), vertices.data());
        glEnableVertexAttribArray(mSpectrogramProgram->attr("point"));
        glVertexAttribPointer(
            mSpectrogramProgram->attr("point"),
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *) 0
        );
        glDrawArrays(GL_LINE_STRIP, 0, vertices.size());
        glDisableVertexAttribArray(mSpectrogramProgram->attr("point"));
    }
}

void OpenGL::renderText(Module::Freetype::Font& font, const std::string& text, int x0, int y0, float r, float g, float b)
{
    if (!font.hasAttachment()) {
        font.setAttachment(new OGL::FontAttachment(font));
    }
    auto fa = font.getAttachment<OGL::FontAttachment>();

    mTextProgram->use();

    glUniformMatrix4fv(mTextProgram->unif("projection"), 1, GL_FALSE, glm::value_ptr(mOrthoProj));
    glUniform3f(mTextProgram->unif("textColor"), r, g, b);
    glActiveTexture(GL_TEXTURE0);

    Module::Freetype::TextRenderData textRenderData = font.prepareTextRender(text);

    for (const auto& glyphRenderData : textRenderData.glyphs) {
        GLfloat x = x0 + glyphRenderData.left;
        GLfloat y = y0 - (glyphRenderData.height - glyphRenderData.top);

        GLfloat w = glyphRenderData.width;
        GLfloat h = glyphRenderData.height;
        
        float vertices[6][4] = {
            { x,     y + h,   0.0f, 0.0f },            
            { x,     y,       0.0f, 1.0f },
            { x + w, y,       1.0f, 1.0f },

            { x,     y + h,   0.0f, 0.0f },
            { x + w, y,       1.0f, 1.0f },
            { x + w, y + h,   1.0f, 0.0f }           
        };

        glBindTexture(GL_TEXTURE_2D, fa->getTextureFor(glyphRenderData.character));

        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glEnableVertexAttribArray(mTextProgram->attr("vertex"));
        glVertexAttribPointer(mTextProgram->attr("vertex"), 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(mTextProgram->attr("vertex"));

        x0 += glyphRenderData.advanceX >> 6;
    }
}

void OpenGL::onDrawableSizeChanged()
{
    resetDrawableSizeChanged();

    int width, height;
    getDrawableSize(&width, &height);

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mMsTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mMsCount, GL_RGBA, width, height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, mMsTexture, 0); 
   
    glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

    glRenderbufferStorageMultisample(GL_RENDERBUFFER, mMsCount, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error(std::string("Renderer::OpenGL] Framebuffer is not complete"));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glViewport(0, 0, width, height);

    mOrthoProj = glm::ortho(0.0f, (float) width, 0.0f, (float) height);
}

GLEWAPIENTRY void OpenGL::debugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *msg, const void *)
{
    std::cout << "OpenGL debug: \033[36m" << msg << "\033[0m" << std::endl;
}
