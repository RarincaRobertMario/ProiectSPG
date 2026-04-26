#pragma once

#ifndef POLICECARPART_H_
#define POLICECARPART_H_

#include "PoliceCarMesh.h"
#include "PoliceCarLight.h"

/**
* @brief Part of a police car.
*/
struct PoliceCarPart 
{
    std::unique_ptr<PoliceCarMesh> meshData = nullptr;      /// Mesh data.
    std::unique_ptr<PoliceCarLight> lightData = nullptr;    /// Light data.
    bool isActivated = false;   /// Flag if the mesh is 'activated'. Needed for light meshes.
};

#endif // POLICECARPART_H_