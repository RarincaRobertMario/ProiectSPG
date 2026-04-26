#pragma once

#ifndef POLICECARPARTINDEX_H_
#define POLICECARPARTINDEX_H_

#include <cstdint>

/**
* @brief Enumeration of car part indexes. These aren't the actual indexes, but you use them to get the actual index of the car part.
* 
* These should be in the same way as you look at the front of the car!
*/
enum class PoliceCarPartIndex : uint8_t
{
    None,           /// Use only if the index doesn't matter (static parts, with no special attributes)
    Body,           /// Body
    HeadLight_L,    /// Headlight Left
    HeadLight_R,    /// Headlight Right
    RearLight_L,    /// Rearlight Left
    RearLight_R,    /// Rearlight Right
    LightBar_L,     /// Lightbar Left
    LightBar_R,     /// Lightbar Right
    HighBeam_L,     /// Highbeam Left
    HighBeam_R,     /// Highbeam Right
    ReverseLight_L, /// Reverse Light Left
    ReverseLight_R, /// Reverse Light Right
    Blinker_FL,     /// Blinker Front Left
    Blinker_FR,     /// Blinker Front Right
    Blinker_BL,     /// Blinker Back Left
    Blinker_BR,     /// Blinker Back Right
    COUNT           /// Number of elements
};

#endif // POLICECARPARTINDEX_H_
