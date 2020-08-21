#ifndef RENDERER_OPENGL_H
#define RENDERER_OPENGL_H

#include "../base/base.h"
#include <GL/glew.h>
#include <unordered_map>

namespace Module::Renderer {

    namespace OGL {
        class FontAttachment {
        public:
            FontAttachment(Module::Freetype::Font& font);

            GLuint getTextureFor(char ch);

        private:
            std::array<GLuint, UCHAR_MAX> mGlyphsTex;
        };

        class ShaderProgram {
        public:
            ShaderProgram(const std::string& shaderName);
            ~ShaderProgram();

            void use();

            GLint attr(const std::string& name);
            GLint unif(const std::string& name);

        private:
            GLuint mID;

            // Using vector-based association maps: faster when there are only a few elements.
            using LocationMap = std::vector<std::pair<std::string, GLint>>;
            LocationMap mAttributes;
            LocationMap mUniforms;

            using LocationGetFn = GLint(*)(GLuint, const GLchar *);
            GLint getLocation(LocationMap& map, const std::string& name, LocationGetFn getFn);
        };
    }

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
        void renderFrequencyTrack(float *track, size_t count) override;

        void renderText(Module::Freetype::Font& font, const std::string& text, int x, int y, float r, float g, float b) override;

    private:
        void onDrawableSizeChanged();

        GLuint createProgram(const std::string& vertexShaderFilename,
                             const std::string& fragmentShaderFilename);

        GLuint loadShader(GLenum shaderType, const std::vector<char>& code);

        static GLEWAPIENTRY void debugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *, const void *);

        OpenGLProvider *mProvider;

        std::vector<Vertex> mVertices;

        OGL::ShaderProgram *mGraphProgram;
        OGL::ShaderProgram *mSpectrogramProgram;
        OGL::ShaderProgram *mTextProgram;

        GLuint mVBO;
        GLuint mVAO;

        GLuint mRBO;
        GLuint mFBO;
        int mMsCount;
        GLuint mMsTexture;
    
        glm::mat4 mOrthoProj;
    };

}

#endif // RENDERER_OPENGL_H
