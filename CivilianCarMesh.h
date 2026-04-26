#pragma once

#ifndef CIVILIANCARMESH_H_
#define CIVILIANCARMESH_H_

#include <memory>

#include "Mesh.h"
#include "CivilianCarMeshType.h"

/**
* @brief Structure for a civilian car mesh.
*/
struct CivilianCarMesh
{
	std::unique_ptr<Mesh> mesh;	/// Mesh of the part.
	glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);	/// Offset of the part.
	CivilianCarMeshType type = CivilianCarMeshType::Body;	/// Type of mesh.
	float lightMeshIntensity = 1.0f;	/// Light mesh intensity.
};

#endif // CIVILIANCARMESH_H_