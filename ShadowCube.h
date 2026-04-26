#pragma once

#ifndef SHADOWCUBE_H_
#define SHADOWCUBE_H_

#include <vector>
#include <glm/glm.hpp>

// Where the units for the shadow cube start.
constexpr unsigned long long int SHADOW_CUBE_UNIT_BASE = 24;

/**
* @brief You heard of ShadowMaps, now get ready for...
* 
* If a ShadowMap is a 2D texture, ShadowCubes are 6 (one for each face) textures!
*/
class ShadowCube 
{
private:
    unsigned int FBO;           /// Frame buffer object. A sort-of screen in GPU memory.
    unsigned int depthCubemap;  /// Depth cubemap, it's 6 textures now!
    unsigned int width;         /// Width of the shadow cube.
    unsigned int height;        /// Height of the shadow cube.

    // No copying.
    ShadowCube(const ShadowCube&) = delete;
    ShadowCube& operator=(const ShadowCube&) = delete;

public:

    /**
    * @brief Constructor
    */
    ShadowCube(unsigned int width = 2048, unsigned int height = 2048);

    /**
    * @brief Move constructor.
    */
    ShadowCube(ShadowCube&& other) noexcept;

    /**
    * @brief Destructor.
    */
    ~ShadowCube();

    /**
    * @brief Binds the shadow cube for writing.
    */
    void BindForWriting() const;

    /**
    * @brief Binds the shadow cube for reading.
    * 
    * @param unit Unit to bind this to.
    */
    void BindForReading(unsigned int unit) const;

    /**
    * @brief Get all shadow matrices for the shader.
    * 
    * @param lightPos Position of the light.
    * @param near The near plane.
    * @param far The far plane.
    * 
    * @return A vector containing 6 4x4 matrices.
    */
    std::vector<glm::mat4> GetShadowMatrices(glm::vec3 lightPos, float near, float far) const;

    /**
    * @brief Returns the ID of the shadow cube.
    * 
    * @return The ID of the shadow cube.
    */
    unsigned int GetCubeID() const;

    /**
    * @brief Returns the width of the shadow cube.
    *
    * @return The width of the shadow cube.
    */
    unsigned int GetWidth() const;

    /**
    * @brief Returns the height of the shadow cube.
    *
    * @return The height of the shadow cube.
    */
    unsigned int GetHeight() const;

    /**
    * @brief Move operator.
    */
    ShadowCube& operator=(ShadowCube&& other) noexcept;
};

#endif // SHADOWCUBE_H_