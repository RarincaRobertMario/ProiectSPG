#include <glad/glad.h>
#include <iostream>
#include "stb_image.h"

#include "Texture.h"

Texture::Texture(const std::string& path, const std::string& uniformName) :
    uniformName(uniformName)
{
    glGenTextures(1, &ID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);

    if (data) 
    {
        GLenum format = (nrComponents == 4) ? GL_RGBA : GL_RGB;

        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else 
    {
        std::cerr << "Texture [" << uniformName << "] failed to load at path : " << path << std::endl;
        stbi_image_free(data);
    }
}

Texture::Texture(uint8_t* data, int width, int height, int components, const std::string& uniformName) :
    uniformName(uniformName)
{
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum format = (components == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::Texture(Texture&& other) noexcept :
    ID(std::exchange(other.ID, 0)),
    uniformName(std::move(other.uniformName)) {}

Texture::~Texture() { glDeleteTextures(1, &ID); }

std::shared_ptr<Texture> Texture::CreateSolidColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, const std::string& uniformName)
{
    // See here how data is a stack variable?
    // Normally this is unsafe, but thankfully OpenGL copies the data to the GPU!
    uint8_t data[] = { r, g, b, a };
    return std::make_shared<Texture>(data, 1, 1, 4, uniformName);
}

void Texture::Bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + TEXTURE_UNIT_BASE + unit);
    glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

const std::string& Texture::GetUniformName() const { return uniformName; }

std::string Texture::GetUniformName() { return uniformName; }

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    glDeleteTextures(1, &ID);

    ID = std::exchange(other.ID, 0);
    uniformName = std::move(other.uniformName);

    return *this;
}
