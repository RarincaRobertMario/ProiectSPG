#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "ShadowMap.h"
#include <iostream>

ShadowMap::ShadowMap(unsigned int width, unsigned int height, float farPlane, float nearPlane, float size, float lightPosOffset, ShadowType type) :
    width(width),
    height(height),
    farPlane(farPlane),
    nearPlane(nearPlane),
    size(size),
    offset(lightPosOffset),
    type(type)
{
    // Generate frame buffer
    glGenFramebuffers(1, &FBO);

    // Generate a texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    // Parameterds (LOD, Linear)
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_LOD_BIAS, 2.0f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Clamp the borders
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Bind the buffer
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

    // Tell OpenGL we aren't painting colors
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Shadow Framebuffer incomplete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMap::ShadowMap(ShadowMap&& other) noexcept :
    FBO(std::exchange(other.FBO, 0)),
    depthMap(std::exchange(other.depthMap, 0)), 
    width(other.width),
    height(other.height),
    farPlane(other.farPlane),
    nearPlane(other.nearPlane),
    size(other.size),
    offset(other.offset),
    up(other.up) {}

ShadowMap::~ShadowMap()
{
    // Cleanup
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &depthMap);
}

void ShadowMap::BindForWriting() const
{
    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::BindForReading(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + SHADOW_MAP_UNIT_BASE + unit);
    glBindTexture(GL_TEXTURE_2D, depthMap);
}

glm::mat4 ShadowMap::GetLightSpaceMatrix(glm::vec3 lightDir, glm::vec3 targetPos, float spotLightOuterAngle) const
{
    glm::vec3 lightPos = targetPos - glm::normalize(lightDir) * offset;

    glm::mat4 lightProjection;

    if (type == ShadowType::Directional) 
    {
        lightProjection = glm::ortho(-size, size, -size, size, nearPlane, farPlane);
    }
    else 
    {
        // Spotlights use perspective (cone)
        float halfAngleRadians = glm::acos(spotLightOuterAngle);

        float fullFOV = halfAngleRadians * 2.0f;

        float finalFOV = fullFOV + glm::radians(20.0f);

        lightProjection = glm::perspective(finalFOV, (float)width / height, 0.1f, farPlane);
    }

    glm::mat4 lightView = glm::lookAt(lightPos, lightPos + lightDir, up);

    return lightProjection * lightView;
}

unsigned int ShadowMap::GetTextureID() const { return depthMap; }

unsigned int ShadowMap::GetWidth() const { return width; }

unsigned int ShadowMap::GetHeight() const { return height; }

ShadowMap& ShadowMap::operator=(ShadowMap&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &depthMap);

    FBO = std::exchange(other.FBO, 0);
    depthMap = std::exchange(other.depthMap, 0);
    width = other.width;
    height = other.height; 
    farPlane = other.farPlane;
    nearPlane = other.nearPlane;
    size = other.size;
    offset = other.offset;
    up = other.up;

    return *this;
}
