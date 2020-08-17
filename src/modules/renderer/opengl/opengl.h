#ifndef RENDERER_OPENGL_H
#define RENDERER_OPENGL_H

#include "../base/base.h"
#include <GL/glew.h>

namespace Module::Renderer {

    class OpenGL : public AbstractBase {
    public:
        OpenGL();
        ~OpenGL();

        void setProvider(void *provider) override; 

        void initialize() override;
        void terminate() override;

        void begin() override;
        void end() override;

        void clear() override;

        void test() override;
 
        void renderGraph(float *x, float *y, size_t count) override;
        void renderSpectrogram(float ***spectrogram, size_t *lengths, size_t count) override;

    private:
        void initMultisampling();

        GLuint loadShader(GLenum shaderType, const std::vector<char>& code);

        OpenGLProvider *mProvider;

        std::vector<Vertex> mVertices;
        UniformBuffer mUniforms;

        GLuint mProgram;
        GLint mLocPosition;
        GLint mLocOffsetX;
        GLint mLocScaleX;

        GLuint mVBO;
        GLuint mVAO;
        GLuint mUBO;

        GLuint mRBO;
        GLuint mFBO;
        int mMsCount;
        GLuint mMsTexture;
    };

}

#endif // RENDERER_OPENGL_H
