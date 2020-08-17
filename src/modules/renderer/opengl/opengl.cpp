#include "opengl.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

using namespace Module::Renderer;

static std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error(std::string("Renderer::Vulkan] Error creating shader module: Could not open file \"") + filename + "\"");
    }

    constexpr size_t codeVectorIncrSize = 4096;
    constexpr size_t readBufferSize = 1024;
    
    auto buffer = std::make_unique<char[]>(readBufferSize);

    std::vector<char> code;

    file.seekg(0);
    do {
        file.read(buffer.get(), readBufferSize);
        size_t bytesRead = file.gcount();
        if (code.size() + bytesRead > code.capacity()) {
            code.reserve(code.capacity() + codeVectorIncrSize);
        }
        code.insert(code.end(), buffer.get(), std::next(buffer.get(), bytesRead));
    } while (file);
    
    code.shrink_to_fit();

    return code;
}

OpenGL::OpenGL()
    : AbstractBase(Type::OpenGL),
      mProvider(nullptr)
{
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
    glDebugMessageCallback([](auto, auto, auto, auto, auto, const GLchar *msg, auto) {
                               std::cout << "OpenGL debug: \033[36m" << msg << "\033[0m" << std::endl;
                           }, nullptr);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD); 
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    // Create FBO and RBO.
    glGenFramebuffers(1, &mFBO);
    glGenRenderbuffers(1, &mRBO);
    
    // Init multisampling.
    glEnable(GL_MULTISAMPLE);
    glGenTextures(1, &mMsTexture);
    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &mMsTexture);
    mMsCount = 4;
    initMultisampling();

    // Create VAO.
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    // Create VBO.
    glGenBuffers(1, &mVBO);

    // Load shaders.
    GLuint shaderVertex = loadShader(GL_VERTEX_SHADER, readFile("shaders/opengl/shader.vert.spv"));
    GLuint shaderFragment = loadShader(GL_FRAGMENT_SHADER, readFile("shaders/opengl/shader.frag.spv"));

    // Create program and link it with the shaders.
    mProgram = glCreateProgram();
    
    glAttachShader(mProgram, shaderVertex);
    glAttachShader(mProgram, shaderFragment);

    glLinkProgram(mProgram);

    GLint isLinked = GL_FALSE;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength;
        glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(mProgram, maxLength, &maxLength, infoLog.data());

        glDeleteProgram(mProgram);
        glDeleteShader(shaderVertex);
        glDeleteShader(shaderFragment);

        throw std::runtime_error(std::string("Renderer::OpenGL] Error loading shader: ") + (const char *) infoLog.data());
    }

    glDetachShader(mProgram, shaderVertex);
    glDetachShader(mProgram, shaderFragment);

    // Find locations.
    mLocPosition = glGetAttribLocation(mProgram, "coord2d");
    mLocOffsetX = glGetUniformLocation(mProgram, "offset_x");
    mLocScaleX = glGetUniformLocation(mProgram, "scale_x");

    // Fill VBO data.
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, 16000 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    mUniforms = {
        .offset_x = 0.0f,
        .scale_x = 1.0f,
    };
}

void OpenGL::terminate()
{
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
    glDeleteFramebuffers(1, &mFBO);
    glDeleteRenderbuffers(1, &mRBO);
    glDeleteProgram(mProgram);
    
    mProvider->deleteContext();
}

void OpenGL::begin()
{
    mProvider->makeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, mRBO);

    if (hasDrawableSizeChanged()) {
        resetDrawableSizeChanged();
        glDeleteTextures(1, &mMsTexture);
        initMultisampling();
    }
    
    glLoadIdentity();

    int width, height;
    getDrawableSize(&width, &height);
    glViewport(0, 0, width, height);
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGL::test()
{
    glUseProgram(mProgram);

    glUniform1f(mLocOffsetX, mUniforms.offset_x);
    glUniform1f(mLocScaleX, mUniforms.scale_x);
    
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);

    glEnableVertexAttribArray(mLocPosition);
    glVertexAttribPointer(
        mLocPosition,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void *) 0
    );

    glDrawArrays(GL_LINE_STRIP, 0, 2000);

    glDisableVertexAttribArray(mLocPosition);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGL::renderGraph(float *x, float *y, size_t count)
{
    glUseProgram(mProgram);

    glUniform1f(mLocOffsetX, mUniforms.offset_x);
    glUniform1f(mLocScaleX, mUniforms.scale_x);
    
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);

    auto graph = std::make_unique<Vertex[]>(count);
    for (int i = 0; i < count; ++i) {
        graph[i].x = 0.9f * (2.0f * (x[i] - x[0]) / (x[count - 1] - x[0]) - 1.0f);
        graph[i].y = y[i] / 10.0f;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(Vertex), graph.get());

    glEnableVertexAttribArray(mLocPosition);
    glVertexAttribPointer(
        mLocPosition,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void *) 0
    );

    glDrawArrays(GL_LINE_STRIP, 0, count);

    glDisableVertexAttribArray(mLocPosition);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGL::renderSpectrogram(float ***spectrogram, size_t *lengths, size_t count)
{
    float minFrequency = 0.0f;
    float maxFrequency = 8000.0f;

    float xfuzz = 0.008f;
    float yfuzz = 0.002f;

    float width = 2.0f / (float) count;

    for (int xi = 0; xi < count; ++xi) {
        float **slice = spectrogram[xi];
        size_t length = lengths[xi];

        float maxSliceFrequency = length > 0 ? slice[length - 1][0] : 0.0f;
        float height = (maxSliceFrequency - minFrequency) / (length * (maxFrequency - minFrequency));

        float x = (2.0f * xi) / (float) count - 1.0f;

        for (int yi = 0; yi < length; ++yi) {
            float y = (2.0f * (slice[yi][0] - minFrequency)) / (maxFrequency - minFrequency) - 1.0f;

            glColor4f(1.0f, 1.0f, 1.0f, slice[yi][1]);
            glRectf(x - 0.5f * xfuzz, y - 0.5f * yfuzz, x + 0.5f * xfuzz + width, y + 0.5f * yfuzz + height);
        }
    }
}

void OpenGL::initMultisampling()
{
    int width, height;
    getDrawableSize(&width, &height);

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, mMsTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, mMsCount, GL_RGB, width, height, GL_TRUE);
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
}

GLuint OpenGL::loadShader(GLenum shaderType, const std::vector<char>& code)
{
    GLuint shaderId = glCreateShader(shaderType);

    glShaderBinary(1, &shaderId, GL_SHADER_BINARY_FORMAT_SPIR_V, code.data(), code.size());

    glSpecializeShader(shaderId, "main", 0, nullptr, nullptr);

    GLint isCompiled = GL_FALSE;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shaderId, maxLength, &maxLength, infoLog.data());

        glDeleteShader(shaderId);
        
        throw std::runtime_error(std::string("Renderer::OpenGL] Error loading shader: ") + (const char *) infoLog.data());
    }

    return shaderId;
}
