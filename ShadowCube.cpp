#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iostream>

#include "ShadowCube.h"

ShadowCube::ShadowCube(unsigned int width, unsigned int height) :
	width(width),
	height(height)
{
    glGenFramebuffers(1, &FBO);

    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    // Create 6 faces for the cube
    for (unsigned int i = 0; i < 6; ++i) 
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_LOD_BIAS, 2.0f);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    // Attach cubemap to the FBO depth buffer
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);

    // No colors!!!
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Shadow Cube-Map Framebuffer not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowCube::ShadowCube(ShadowCube&& other) noexcept :
    FBO(std::exchange(other.FBO, 0)),
    depthCubemap(std::exchange(other.depthCubemap, 0)),
    width(other.width),
    height(other.height) {}

ShadowCube::~ShadowCube()
{
    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &depthCubemap);
}

void ShadowCube::BindForWriting() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, width, height);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowCube::BindForReading(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + SHADOW_CUBE_UNIT_BASE + unit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
}

std::vector<glm::mat4> ShadowCube::GetShadowMatrices(glm::vec3 lightPos, float near, float far) const
{
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)width / (float)height, near, far);

    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)));

    return shadowTransforms;
}

unsigned int ShadowCube::GetCubeID() const { return depthCubemap; }

unsigned int ShadowCube::GetWidth() const { return width; }

unsigned int ShadowCube::GetHeight() const { return height; }

ShadowCube& ShadowCube::operator=(ShadowCube&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    glDeleteFramebuffers(1, &FBO);
    glDeleteTextures(1, &depthCubemap);

    FBO = std::exchange(other.FBO, 0);
    depthCubemap = std::exchange(other.depthCubemap, 0);
    width = other.width;
    height = other.height;

    return *this;
}