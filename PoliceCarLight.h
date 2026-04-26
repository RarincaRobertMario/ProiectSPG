#pragma once

#ifndef POLICECARLIGHT_H_
#define POLICECARLIGHT_H_

#include <variant>

#include "PoliceCarLightType.h"
#include "PointLight.h"
#include "ObjectPosition.h"
#include "SpotLight.h"

/**
* @brief Structure for a police car.
*/
struct PoliceCarLight
{
    std::unique_ptr<PointLight> pointLight; /// Point light.
    std::unique_ptr<SpotLight> spotLight;   /// Spot light.
    glm::vec3 offset{ 0.0f, 0.0f, 0.0f };   /// Position relative to car.
    bool flashing = false;      /// Whether it pulses or not.
    PoliceCarLightType type = PoliceCarLightType::LightBar;    /// Kind of light.
    ObjectPosition position{};  /// Position of the light relative to the car.
    float pulseSpeed = 1.0f;    /// Flash speed. Ignored if set to not flash.
};

#endif // POLICECARLIGHT_H_