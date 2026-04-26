#pragma once

#ifndef SPOTLIGHT_H_
#define SPOTLIGHT_H_

#include <glm/glm.hpp>

#include "LightProperties.h"
#include "ShadowMapIndex.h"

class Shader;

/**
 * @brief Represents a spotlight in the scene.
 * 
 * Between [cutOff - outerCutOff] the light is interpolated.
 */
struct SpotLight
{
    LightProperties base = {};     /// Light properties.

    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);   /// Direction of the spotlight.

    float cutOff =  glm::cos(glm::radians(25.0f));      /// Inner cut-off of the spotlight, between 0 and 1.
    float outerCutOff = glm::cos(glm::radians(45.0f));  /// Outer cut-off of the spotlight, between 0 and 1.
    ShadowMapIndex shadowMapIdx = ShadowMapIndex::None;   /// Shadow map index.

    /**
    * @brief Applies the light on a shader.
    *
    * @param shader Reference to the shader.
    * @param index Which index in the vector of lights this is.
    */
    void Apply(const Shader& shader, int index) const;
};

#endif // SPOTLIGHT_H_