#pragma once

#ifndef SHADOWMAP_H_
#define SHADOWMAP_H_

#include <glm/glm.hpp>

#include "ShadowType.h"

// Where the units for the shadow map start.
constexpr unsigned long long int SHADOW_MAP_UNIT_BASE = 10;

/**
* @brief Class for a shadow map.
*/
class ShadowMap 
{
private:
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); /// The direction of 'up'. If a light is directly overhead, change to (0, 0, 1).
    unsigned int FBO;           /// Frame buffer object. A sort-of screen in GPU memory.
    unsigned int depthMap;      /// Depth map, it's actually a texture.
    unsigned int width;         /// Width.
    unsigned int height;        /// Height.
    float farPlane;             /// Far Plane.
    float nearPlane;            /// Near Plane.
    float size;                 /// The size of the shadow map.
    float offset;               /// Offset of the light.
    ShadowType type = ShadowType::Directional;  /// Type of light.

    // No copying.
    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;

public:

    /**
    * @brief Constructor.
    * 
    * Think of a shadow map as a cube of sizeXsize that starts at nearPlane and ends at farPlane.
    * And everything inside said cube has a shadow.
    * 
    * @param width Wdith of the shadow map.
    * @param height Height of the shadow map.
    * @param farPlane The far plane of the shadow map.
    * @param nearPlane The near plane of the shadow map.
    * @param size The size of the shadow map.
    * @param lightPosOffset The offset of light position. Where it is in relation to the cube.
    * @param type Type of shadow (direction or spotlight).
    */
    ShadowMap(
        unsigned int width = 2048,
        unsigned int height = 2048,
        float farPlane = 350.0f,
        float nearPlane = 100.0f,
        float size = 50.0f,
        float lightPosOffset = 200.0f,
        ShadowType type = ShadowType::Directional
    );

    /**
    * @brief Move constructor.
    */
    ShadowMap(ShadowMap&& other) noexcept;

    /**
    * @brief Destructor.
    */
    ~ShadowMap();

    /**
    * @brief Binds the shadow map for writing.
    * 
    * Call before rendering the shadow pass.
    */
    void BindForWriting() const;

    /**
    * @brief Binds the shadow map for reading.
    * 
    * Call this before the main lighting pass.
    * 
    * @param unit Unit to attach this to.
    */
    void BindForReading(unsigned int unit) const;

    /**
    * @brief Returns the light space matrix.
    * 
    * @param lightDir Direction of light.
    * @param targetPos Target position.
    * @param spotLightOuterAngle If the shadow map is for a spotlight, this is the outer angle of the spotlight, between 0 and 1, as defined in the Spotlight struct.
    * 
    * @return A 4X4 light space matrix.
    */
    glm::mat4 GetLightSpaceMatrix(glm::vec3 lightDir, glm::vec3 targetPos, float spotLightOuterAngle) const;

    /**
    * @brief Returns the texture ID.
    * 
    * @return The texture ID.
    */
    unsigned int GetTextureID() const;

    /**
    * @brief Returns the width of the shadow map.
    * 
    * @return The width of the shadow map.
    */
    unsigned int GetWidth() const;

    /**
    * @brief Returns the height of the shadow map.
    * 
    * @return The height of the shadow map.
    */
    unsigned int GetHeight() const;

    /**
    * @brief Move operator.
    */
    ShadowMap& operator=(ShadowMap&& other) noexcept;
};

#endif // SHADOWMAP_H_