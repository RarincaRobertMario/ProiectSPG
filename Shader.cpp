#include <glad/glad.h>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Shader.h"

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath)
{
    std::string vertexCode, fragmentCode, geometryCode;
    // Open the files.
    std::ifstream vFile(vertexPath);
    std::ifstream fFile(fragmentPath);
    if (!vFile.is_open() || !fFile.is_open())
    {
        std::cerr << "{Shader::Shader} [ERROR] Cannot open shader files: " << vertexPath << " or " << fragmentPath << std::endl;
        return;
    }

    // Get the string streams.
    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();
    vFile.close();
    fFile.close();

    // Get the strings.
    vertexCode = vStream.str();
    fragmentCode = fStream.str();

    const char* gCodePtr = nullptr;
    if (!geometryPath.empty()) 
    {
        std::ifstream gFile(geometryPath);
        if (gFile.is_open()) 
        {
            std::stringstream gStream;
            gStream << gFile.rdbuf();
            geometryCode = gStream.str();
            gCodePtr = geometryCode.c_str();
        }
        else
        {
            std::cerr << "{Shader::Shader} [ERROR] Cannot open geometry shader files: " << geometryPath << std::endl;
        }
    }

    LoadShaders(vertexCode.c_str(), fragmentCode.c_str(), gCodePtr);
}

Shader::Shader(Shader&& other) noexcept :
    ID(std::exchange(other.ID, 0)),
    uniformCache(std::move(other.uniformCache)),
    shaderName(std::move(other.shaderName)) {}

Shader::~Shader()
{
    glDeleteProgram(ID);
}

int Shader::GetUniformLocation(const std::string& name) const
{
    if (uniformCache.find(name) != uniformCache.end())
    {
        return uniformCache[name];
    }

    int loc = glGetUniformLocation(ID, name.c_str());
    if (loc == -1)
    {
        std::cerr << "{Shader::GetUniformLocation} Warning for [" << (shaderName.empty() ? "NO-NAME" : shaderName) << "]: uniform '" << name << "' not found!" << std::endl;
    }

    uniformCache[name] = loc;
    return loc;
}

void Shader::UseShader() const { glUseProgram(ID); }

unsigned int Shader::GetID() const { return ID; }

void Shader::SetName(const std::string& newName) { shaderName = newName; }

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    glDeleteProgram(ID);

    ID = std::exchange(other.ID, 0);
    uniformCache = std::move(other.uniformCache);
    shaderName = std::move(other.shaderName);

    return *this;
}

void Shader::CheckShaderCompile(unsigned int shader, const std::string& type, const std::string& shaderName)
{
    int success;
    char infoLog[1024];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, infoLog);
        std::cerr << "[ERROR] [" << type << "] SHADER [" << (shaderName.empty() ? "NO-NAME" : shaderName) << "] COMPILATION FAILED : " << infoLog << std::endl;
    }
}

void Shader::CheckProgramLink(unsigned int program, const std::string& shaderName)
{
    int success;
    char infoLog[1024];

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 1024, NULL, infoLog);
        std::cerr << "[ERROR] PROGRAM LINKING FOR SHADER [" << (shaderName.empty() ? "NO-NAME" : shaderName) << "] FAILED: " << infoLog << std::endl;
    }
}

void Shader::LoadShaders(const char* vertexSrc, const char* fragmentSrc, const char* geometrySrc)
{
    // Compile the vertex shader.
    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSrc, NULL);
    glCompileShader(vertex);
    CheckShaderCompile(vertex, "VERTEX", shaderName);

    // Compile the fragment shader.
    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSrc, NULL);
    glCompileShader(fragment);
    CheckShaderCompile(fragment, "FRAGMENT", shaderName);

    // Compile geometry shader (needed for shadow cubes)
    unsigned int geometry = 0;
    if (geometrySrc) 
    {
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &geometrySrc, NULL);
        glCompileShader(geometry);
        CheckShaderCompile(geometry, "GEOMETRY", shaderName);
    }

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (geometrySrc) 
    {
        glAttachShader(ID, geometry);
    }
    glLinkProgram(ID);

    CheckProgramLink(ID, shaderName);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (geometrySrc) 
    {
        glDeleteShader(geometry);
    }
}
