#pragma once

#ifndef BLINKERSIDE_H_
#define BLINKERSIDE_H_

#include <cstdint>

/**
* @brief Enumeration of which blinker lights are on.
*
* Position as you're looking at the front of the car.
*/
enum class BlinkerSide : uint8_t
{
    None,           /// None.
    LeftSide,       /// Left side.
    RightSide,      /// Right side.
    HazardLights,   /// All
    COUNT           /// Number of elements.
};

#endif // BLINKERSIDE_H_