#pragma once

#ifndef SHADOWMAPINDEX_H_
#define SHADOWMAPINDEX_H_

#include <cstdint>

/**
* @brief Enumeration of shadow map indexes.
* 
* These should match the same values as defined in the fragment shader (inside the spotlight loop)
*/
enum class ShadowMapIndex : int8_t
{
    None = -1,              /// Not using a shadow map.
    PL_Headlight_L = 0,     /// Shadow map for the police car's left headlight.
    PL_Headlight_R = 1,     /// Shadow map for the police car's right headlight.
    PL_Rearlight_L = 2,     /// Shadow map for the police car's left rear light.
    PL_Rearlight_R = 3,     /// Shadow map for the police car's right rear light.
    PL_Reverselight_L = 4,  /// Shadow map for the police car's left reverse light.
    PL_Reverselight_R = 5,  /// Shadow map for the police car's right reverse light.
    PL_Highbeam_L = 6,      /// Shadow map for the police car's left high beam.
    PL_Highbeam_R = 7,      /// Shadow map for the police car's right high beam.
    CV_Headlight_L = 8,     /// Shadow map for the civilian car's right headlight.
    CV_Headlight_R = 9,     /// Shadow map for the civilian car's right headlight.
    CV_Rearlight_L = 10,    /// Shadow map for the civilian car's left rear light.
    CV_Rearlight_R = 11,    /// Shadow map for the civilian car's right rear light.
    COUNT                   /// Number of elements.
};


#endif // SHADOWMAPINDEX_H_