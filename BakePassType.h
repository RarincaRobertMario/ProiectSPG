#pragma once

#ifndef BAKEPASSTYPE_H_
#define BAKEPASSTYPE_H_

/**
* @brief Enumeration of bake pass types.
*/
enum class BakePassType
{
	Solid,	/// Default bake pass for static, non-transparent meshes.
	Alpha,		/// Pass for transparent meshes.
	Light,		/// Pass for light-related meshes.
	COUNT		/// Number of elements.
};

#endif // BAKEPASSTYPE_H_