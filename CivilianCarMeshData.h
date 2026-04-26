#pragma once

#ifndef CIVILIANCARMESHDATA_H_
#define CIVILIANCARMESHDATA_H_

#include <memory>

#include "Mesh.h"

/**
* @brief Civilian car mesh data.
*/
struct CivilianCarMeshData
{
	std::unique_ptr<Mesh> bakedBodyMesh;	/// Baked body mesh.
	std::unique_ptr<Mesh> bakedAlphaMesh;	/// Baked alpha mesh.
	std::unique_ptr<Mesh> bakedLightMesh;	/// Baked light mesh.
	std::shared_ptr<Texture> windowTex;		/// Shared window texture.
};

#endif // CIVILIANCARMESHDATA_H_