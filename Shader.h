#pragma once

#ifndef SHADER_H_
#define SHADER_H_

#include <string>
#include <unordered_map>

/**
* @brief Encapsulates a shader of OpenGL using RAII.
*/
class Shader 
{
private:
    std::string shaderName; /// Optional shader name.
    unsigned int ID = 0;    /// ID of the shader.
    mutable std::unordered_map<std::string, int> uniformCache;  /// Uniform cache.

    // No copying.
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    /**
    * @brief Checks if a shader has compiled successfully or not.
    * 
    * @param shader The ID of the shader to check.
    * @param type The type of shader used (vertex, fragment).
    * @param shaderName Shader name, for debug.
    */
    static void CheckShaderCompile(unsigned int shader, const std::string& type, const std::string& shaderName = "");

    /**
    * @brief Checks if a program has linked correctly.
    *
    * @param program The ID of the program.
    * @param shaderName Shader name, for debug.
    */
    static void CheckProgramLink(unsigned int program, const std::string& shaderName = "");

    /**
    * @brief Loads the shaders.
    * 
    * @param vertexSrc String containing the source of the vertex shader.
    * @param fragmentSrc String containing the source of the fragment shader.
    * @param geometrySrc String containing the source of the geometry shader.
    */
    void LoadShaders(const char* vertexSrc, const char* fragmentSrc, const char* geometrySrc = nullptr);

public:

    /**
    * @brief Constructor.
    *
    * @param vertexPath String containing the path to the vertex shader.
    * @param fragmentPath String containing the path to the fragment shader.
    * @param geometryPath String containing the path to the geometry shader.
    */
    Shader(
        const std::string& vertexPath,
        const std::string& fragmentPath,
        const std::string& geometryPath = ""
    );

    /**
    * @brief Move constructor.
    */
    Shader(Shader&& other) noexcept;

    /**
    * @brief Deletes the shader.
    */
    ~Shader();

    /**
    * @brief Returns the uniform location of a uniform parameter.
    * 
    * @param name Name of the uniform parameter.
    * 
    * @return The location of the uniform parameter, or -1 if no such parameter exists.
    */
    int GetUniformLocation(const std::string& name) const;

    /**
    * @brief Tells OpenGL to use this specific shader.
    */
    void UseShader() const;

    /**
    * @brief Returns the ID of the shader.
    * 
    * @return The ID of the shader.
    */
    unsigned int GetID() const;

    /**
    * @brief Sets the name of the shader.
    *
    * Has no impact on shading, just for debug.
    * 
    * @param newName New name of the shader.
    */
    void SetName(const std::string& newName);

    /**
    * @brief Move operator.
    */
    Shader& operator=(Shader&& other) noexcept;
};

#endif // SHADER_H_