#pragma once

#ifndef CITYMESHDATA_H_
#define CITYMESHDATA_H_

#include "Mesh.h"
#include "Texture.h"

/**
* @brief Structure for the city mesh data.
*/
struct CityMeshData
{
    std::unique_ptr<Mesh> bakedBodyMesh;    /// Baked opaque mesh.
    std::unique_ptr<Mesh> bakedAlphaMesh;   /// Baked meshes with alpha modulation.
    std::unique_ptr<Mesh> bakedLightMesh;   /// Bakes meshes for lights.
    std::shared_ptr<Texture> windowTex;     /// Shared window texture.
};

#endif // CITYMESHDATA_H_