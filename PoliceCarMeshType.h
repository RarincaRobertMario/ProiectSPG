#pragma once

#ifndef POLICECARMESHTYPE_H_
#define POLICECARMESHTYPE_H_

#include <cstdint>

/**
* @brief Enumeration of mesh types for a police car.
*/
enum class PoliceCarMeshType : uint8_t
{
    Body,       /// Body of the car.
    Light,      /// Lights of the car.
    Wheel,      /// Wheels.
    Steering,   /// Steering wheel.
    Window,     /// Window (transparent).
    Other,      /// Anything else.
    COUNT       /// Number of elements.
};

#endif // POLICECARMESHTYPE_H_