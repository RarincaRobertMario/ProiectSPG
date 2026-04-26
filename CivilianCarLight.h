#pragma once

#ifndef CIVILIANCARLIGHT_H_
#define CIVILIANCARLIGHT_H_

#include "SpotLight.h"
#include "CivilianCarLightType.h"
#include "ObjectPosition.h"

/**
* @brief Structure for a civilian light.
*/
struct CivilianCarLight
{
    std::unique_ptr<SpotLight> spotLight;   /// Spot light.
    glm::vec3 offset{ 0.0f, 0.0f, 0.0f };   /// Position relative to car.
    CivilianCarLightType type = CivilianCarLightType::Headlight;    /// Kind of light.
    ObjectPosition position = {};   /// Position of the light.
};

#endif // CIVILIANCARLIGHT_H_