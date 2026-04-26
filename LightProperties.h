#pragma once

#ifndef LIGHT_H_
#define LIGHT_H_

#include <glm/glm.hpp>

/**
* @brief Properties for a light.
*/
struct LightProperties
{
    glm::vec3 position{ 0.0f, 0.0f, 0.0f }; /// Position of the light.
    glm::vec3 baseColor{ 1.0f, 1.0f, 1.0f };    /// Color of the light, ideally this doesn't change too often.
    glm::vec3 currentColor{ 1.0f, 1.0f, 1.0f }; /// Current color of the light.

    float constant = 1.0f;      /// Constant attenuation.
    float linear = 0.09f;       /// Linear attenuation.
    float quadratic = 0.032f;   /// Quadratic attenuation.

    float intensity = 1.0f;     /// Strength of the light.
};

#endif // LIGHT_H_