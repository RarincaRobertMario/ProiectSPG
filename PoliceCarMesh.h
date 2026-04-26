#pragma once

#ifndef POLICECARMESH_H_
#define POLICECARMESH_H_

#include "Mesh.h"
#include "PoliceCarMeshType.h"
#include "ObjectPosition.h"

/**
* @brief Structure for a police car's mesh.
*/
struct PoliceCarMesh
{
    std::unique_ptr<Mesh> mesh;  /// The actual mesh.
    glm::vec3 offset{ 0.0f, 0.0f, 0.0f };   /// Position relative to center of the car.
    PoliceCarMeshType type = PoliceCarMeshType::Other;    /// What kind of mesh.
    ObjectPosition position{};  /// Position of the mesh relative to the car.
    float lightMeshIntensity = 0.0f;    /// If the mesh is a light-type, give the light intensity.
                                        /// Ignored if mesh type is anything else.
    bool castShadow = true;     /// Flag if the mesh should cast a shadow.
    bool isTransparent = false; /// Flag if the mesh is transparent or not.
};

#endif // POLICECARMESH_H_