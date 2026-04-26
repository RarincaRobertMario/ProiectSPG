#pragma once

#ifndef POINTLIGHT_H_
#define POINTLIGHT_H_

#include "LightProperties.h"
#include "ShadowCubeIndex.h"

class Shader;

/**
* @brief A structure for a point light.
*/
struct PointLight
{
    LightProperties base; /// Light properties.
    ShadowCubeIndex shadowCubeIdx = ShadowCubeIndex::None;  /// Shadow cube index.

    /**
    * @brief Applies the light on a shader.
    *
    * @param shader Reference to the shader.
    * @param index Which index in the vector of lights this is.
    */
    void Apply(const Shader& shader, int index) const;
};

#endif // POINTLIGHT_H_