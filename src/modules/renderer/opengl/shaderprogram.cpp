#include "opengl.h"
#include <fstream>
#include <iostream>
#include <memory>

using namespace Module::Renderer;

static std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error(std::string("Renderer::OpenGL] Error reading shader file \"") + filename + "\"");
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

static GLuint loadShader(GLenum shaderType, const std::vector<char>& code)
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

OGL::ShaderProgram::ShaderProgram(const std::string& shaderName)
{
    mID = glCreateProgram();
    
    const std::string vertexShaderFilename("shaders/opengl/" + shaderName + ".vert.spv");
    const std::string fragmentShaderFilename("shaders/opengl/" + shaderName + ".frag.spv");

    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, readFile(vertexShaderFilename));
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, readFile(fragmentShaderFilename));

    glAttachShader(mID, vertexShader);
    glAttachShader(mID, fragmentShader);

    glLinkProgram(mID);

    GLint isLinked = GL_FALSE;
    glGetProgramiv(mID, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength;
        glGetProgramiv(mID, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(mID, maxLength, &maxLength, infoLog.data());

        glDeleteProgram(mID);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        throw std::runtime_error(std::string("Renderer::OpenGL] Error creating program: ") + (const char *) infoLog.data());
    }

    glDetachShader(mID, vertexShader);
    glDetachShader(mID, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    mAttributes.reserve(10);
    mUniforms.reserve(10);
}

OGL::ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(mID);
}

void OGL::ShaderProgram::use()
{
    glUseProgram(mID);
}

GLint OGL::ShaderProgram::attr(const std::string& name)
{
    return getLocation(mAttributes, name, glGetAttribLocation);
}

GLint OGL::ShaderProgram::unif(const std::string& name)
{
    return getLocation(mAttributes, name, glGetUniformLocation);
}

GLint OGL::ShaderProgram::getLocation(LocationMap& map, const std::string& name, LocationGetFn getFn)
{
    for (const auto& [itName, itLoc] : map) {
        if (itName == name)
            return itLoc;
    }
    GLint loc = getFn(mID, name.c_str());
    if (loc == -1) {
        std::cout << "Renderer::OpenGL] Shader variable " << name << " not found." << std::endl;
    }
    map.emplace_back(name, loc);
    return loc;
}
