#ifndef RENDERER_GLES_H
#define RENDERER_GLES_H

#include "../base/base.h"

#define GL_ES_VERSION 
#include <GLES3/gl32.h>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2ext.h>

namespace Module::Renderer {

    class GLES : public AbstractBase {
    public:
        GLES();
        ~GLES();

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
        GLuint createProgram(const std::string& vertexShaderFilename,
                             const std::string& fragmentShaderFilename);

        GLuint loadShader(GLenum shaderType, const std::vector<char>& code);

        static GL_APIENTRY void debugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *, const void *);

        OpenGLProvider *mProvider;

        std::vector<Vertex> mVertices;
        UniformBuffer mUniforms;

        GLuint mGraphProgram;
        GLint mLocPosition;
        GLint mLocOffsetX;
        GLint mLocScaleX;

        GLuint mSpectrogramProgram;
        GLint mLocPoint;
        GLint mLocMinFreq;
        GLint mLocMaxFreq;
        GLint mLocScaleType;

        GLuint mVBO;
        GLuint mVAO;
    };

}

#endif // RENDERER_GLES_H
