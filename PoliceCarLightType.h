#pragma once

#ifndef POLICECARLIGHTYPE_H_
#define POLICECARLIGHTYPE_H_

#include <cstdint>

/**
* @brief Light type on a police car.
*/
enum class PoliceCarLightType : uint8_t
{
    LightBar,       /// Light bar.
    Headlight,      /// Head light.
    HighBeam,       /// High beam light.
    BrakeLight,     /// Brake light.
    Blinker,        /// Blinker light.  
    ReverseLight,   /// Reverse light.
    COUNT           /// Number of elements.
};
#endif // POLICECARLIGHTYPE_H_