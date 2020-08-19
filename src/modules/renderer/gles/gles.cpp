#include "gles.h"
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
        throw std::runtime_error(std::string("Renderer::GLES] Error reading shader file \"") + filename + "\"");
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

GLES::GLES()
    : AbstractBase(Type::GLES),
      mProvider(nullptr)
{
}

GLES::~GLES()
{
}

void GLES::setProvider(void *provider)
{
    mProvider = static_cast<OpenGLProvider *>(provider);
}

void GLES::initialize()
{
    mProvider->createContext();

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLES::debugCallback, nullptr);

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD); 
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    
    // Create VAO.
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    // Create VBO.
    glGenBuffers(1, &mVBO);

    // Create graph program.
    mGraphProgram = createProgram("shaders/gles/graph.vert", "shaders/gles/graph.frag");
    mLocPosition = glGetAttribLocation(mGraphProgram, "coord2d");
    mLocOffsetX = glGetUniformLocation(mGraphProgram, "offset_x");
    mLocScaleX = glGetUniformLocation(mGraphProgram, "scale_x");

    // Create spectrogram program.
    mSpectrogramProgram = createProgram("shaders/gles/spectrogram.vert", "shaders/gles/spectrogram.frag");
    mLocPoint = glGetAttribLocation(mSpectrogramProgram, "point");
    mLocMinFreq = glGetUniformLocation(mSpectrogramProgram, "minFreq");
    mLocMaxFreq = glGetUniformLocation(mSpectrogramProgram, "maxFreq");
    mLocScaleType = glGetUniformLocation(mSpectrogramProgram, "scaleType");

    // Fill VBO data.
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, 16000 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

    mUniforms = {
        .offset_x = 0.0f,
        .scale_x = 1.0f,
    };
}

void GLES::terminate()
{
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
    glDeleteProgram(mGraphProgram);
    glDeleteProgram(mSpectrogramProgram);
    
    mProvider->deleteContext();
}

void GLES::begin()
{
    mProvider->makeCurrent();
    
    int width, height;
    getDrawableSize(&width, &height);
    glViewport(0, 0, width, height);
}

void GLES::end()
{
    mProvider->swapTarget();
}

void GLES::clear()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLES::test()
{
    glUseProgram(mGraphProgram);

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
}

void GLES::renderGraph(float *x, float *y, size_t count)
{
    glUseProgram(mGraphProgram);

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
}

void GLES::renderSpectrogram(float ***spectrogram, size_t *lengths, size_t count)
{
    glUseProgram(mSpectrogramProgram);

    glUniform1f(mLocMinFreq, 0.0f);
    glUniform1f(mLocMaxFreq, 4000.0f);
    glUniform1ui(mLocScaleType, 2);

    float xfuzz = 0.008f;
    float yfuzz = 0.002f;

    float width = 2.0f / (float) count;

    for (int xi = 0; xi < count; ++xi) {
        float **slice = spectrogram[xi];
        size_t length = lengths[xi];

        float x = (2.0f * xi) / (float) count - 1.0f;

        auto sliceVertices = std::make_unique<glm::vec3[]>(length);

        for (int yi = 0; yi < length; ++yi) {
            sliceVertices[yi][0] = x;
            sliceVertices[yi][1] = slice[yi][0];
            sliceVertices[yi][2] = slice[yi][1];
        }
    
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);

        glBufferSubData(GL_ARRAY_BUFFER, 0, length * sizeof(glm::vec3), sliceVertices.get());

        glEnableVertexAttribArray(mLocPoint);
        glVertexAttribPointer(
            mLocPoint,
            3,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *) 0
        );

        glDrawArrays(GL_LINE_STRIP, 0, length);

        glDisableVertexAttribArray(mLocPoint);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

GLuint GLES::createProgram(const std::string& vertexShaderFilename,
                           const std::string& fragmentShaderFilename)
{
    GLuint program = glCreateProgram();
    
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, readFile(vertexShaderFilename));
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, readFile(fragmentShaderFilename));

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    GLint isLinked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());

        glDeleteProgram(program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        throw std::runtime_error(std::string("Renderer::GLES] Error creating program: ") + (const char *) infoLog.data());
    }

    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint GLES::loadShader(GLenum shaderType, const std::vector<char>& code)
{
    GLuint shaderId = glCreateShader(shaderType);

    const GLchar *string = code.data();
    const GLint length = code.size();

    glShaderSource(shaderId, 1, &string, &length);

    glCompileShader(shaderId);

    GLint isCompiled = GL_FALSE;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shaderId, maxLength, &maxLength, infoLog.data());

        glDeleteShader(shaderId);
        
        throw std::runtime_error(std::string("Renderer::GLES] Error loading shader: ") + (const char *) infoLog.data());
    }

    return shaderId;
}

GL_APIENTRY void GLES::debugCallback(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *msg, const void *)
{
    std::cout << "GLES debug: \033[36m" << msg << "\033[0m" << std::endl;
}
