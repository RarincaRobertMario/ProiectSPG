#pragma once

#ifndef CIVILIANCARMESHTYPE_H_
#define CIVILIANCARMESHTYPE_H_

/**
* @brief Enumeration of mesh types for the civilian car.
*/
enum class CivilianCarMeshType
{
	Body,		/// Body mesh.
	Light,		/// Light mesh.
	Wheel,		/// Wheel mesh.
	Steering,	/// Steering mesh.
	Window,		/// Window mesh.
	COUNT		/// Number of elements.
};

#endif // CIVILIANCARMESHTYPE_H_