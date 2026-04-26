#pragma once

#ifndef POLICECARMODELDATA_H_
#define POLICECARMODELDATA_H_

#include <vector>
#include <array>
#include <memory>

#include "PoliceCarPart.h"
#include "PoliceCarPartIndex.h"

/**
* @brief Welcome back flyweight, since this should be shared as much, as long as the models are static.
* 
* A data structure for a police car mode.
*/
struct PoliceCarModelData 
{
    std::vector<PoliceCarPart> parts;   /// Vector of parts.
    std::array<long long int, static_cast<size_t>(PoliceCarPartIndex::COUNT)> carPartIndex = { -1 }; /// Array of car part indexes.
    std::unique_ptr<Mesh> bakedBodyMesh;   /// Baked opaque mesh.
    std::unique_ptr<Mesh> bakedAlphaMesh;  /// Baked meshes with alpha modulation.
    std::shared_ptr<Texture> windowTex;    /// Shared window texture.
};

#endif // POLICECARMODELDATA_H_