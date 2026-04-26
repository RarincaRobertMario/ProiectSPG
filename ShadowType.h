#pragma once

#ifndef SHADOWTYPE_H_
#define SHADOWTYPE_H_

#include <cstdint>

/**
* @brief Enumeration of shadow map types.
*/
enum class ShadowType : uint8_t
{
    Directional,    /// Direction light (infinite direction, no 'real position')
    Spot,           /// Spot lights (has finite direction, has a real position)
    COUNT           /// Number of elements.
};

#endif // SHADOWTYPE_H_