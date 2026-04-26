#pragma once

#ifndef SHADOWCUBEINDEX_H_
#define SHADOWCUBEINDEX_H_

#include <cstdint>

enum class ShadowCubeIndex : int8_t
{
	None = -1,
	Lightbar_L = 0,
	Lightbar_R = 1,
	Blinker_FL = 2,
	Blinker_FR = 3,
	Blinker_BL = 4,
	Blinker_BR = 5,
	COUNT			/// Number of elements.
};

#endif // SHADOWCUBEINDEX_H_