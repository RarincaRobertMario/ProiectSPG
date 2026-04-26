#pragma once

#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <string>

// Where the units for the textures start.
constexpr unsigned long long int TEXTURE_UNIT_BASE = 0;

/**
* @brief A class for a texture.
*/
class Texture 
{
private:
    unsigned int ID;            /// ID of the texture.
    std::string uniformName;    /// Uniform name of the texture.

    // No copying.
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

public:

    /**
    * @brief Constructor. Loads a texture
    * 
    * @param path Path to the file containing the texture.
    * @param uniformName Uniform name of the texture.
    */
    Texture(const std::string& path, const std::string& uniformName);

    /**
    * @brief Constructor. Creates texture from raw data.
    * 
    * @param data Data of the texture.
    * @param width Width of the texture.
    * @param height Height of the texture.
    * @param components Components of the texture (RGB, RGBA)
    * @param uniformName Uniform name of the texture.
    */
    Texture(uint8_t* data, int width, int height, int components, const std::string& uniformName);

    /**
    * @brief Move constructor.
    */
    Texture(Texture&& other) noexcept;

    /**
    * @brief Destructor.
    */
    ~Texture();

    /**
    * @brief Creates a solid color texture.
    * 
    * @param r Red component.
    * @param g Green component.
    * @param b Blue component.
    * @param a Alpha component.
    * @param uniformName Uniform name of the texture.
    */
    static std::shared_ptr<Texture> CreateSolidColor(
        uint8_t r, 
        uint8_t g,
        uint8_t b,
        uint8_t a,
        const std::string& uniformName
    );

    /**
    * @brief Binds a texture to a certain unit.
    */
    void Bind(unsigned int unit = 0) const;

    /**
    * @brief Unbinds the texture.
    */
    void Unbind() const;

    /**
    * @brief Returns the uniform name of the texture.
    * 
    * @return Constant reference to the uniform name of the texture.
    */
    const std::string& GetUniformName() const;

    /**
    * @brief Returns the uniform name of the texture.
    *
    * @return The uniform name of the texture.
    */
    std::string GetUniformName();

    /**
    * @brief Move operator.
    */
    Texture& operator=(Texture&& other) noexcept;
};

#endif // TEXTURE_H_