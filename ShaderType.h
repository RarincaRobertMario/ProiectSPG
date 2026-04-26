#pragma once

#ifndef SHADERTYPE_H_
#define SHADERTYPE_H_

#include <cstdint>

/**
* @brief Enumeration of shader types.
*/
enum class ShaderType : uint8_t
{
	Phong,			/// Default phong shader.
	ShadowMap,		/// Shadow map shader.
	ShadowCube,		/// Shadow cube shader.
	Xray,			/// Xray shader.
	COUNT			/// Number of elements.
};

#endif // SHADERTYPE_H_