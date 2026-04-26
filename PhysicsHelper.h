#pragma once

#ifndef PHYSICSHELPER_H_
#define PHYSICSHELPER_H_

#include <vector>

#include "BoundingBox.h"
#include "Mesh.h"

/**
* @brief Namespace for physics related helper functions.
*/
namespace PhysicsHelper 
{
    /**
    * @brief Calculates the bounding box (Axis-Aligned Bounding Box - AABB) of a mesh.
    * 
    * This is a simple min-max bounding box, it does not rotate on its own, instead if resizes itself to fit if needed.
    * 
    * @param vertices Vertices of the mesh.
    * @param stride Stride of the mesh.
    */
    BoundingBox CalculateAABB(const std::vector<float>& vertices, int stride = STRIDE);

    /**
    * @brief Gets the height (Y) on a triangle.
    * 
    * @param p1 The first edge of the triangle.
    * @param p2 The second edge of the triangle.
    * @param p3 The third edge of the triangle.
    * @param x The X coordinate of the point.
    * @param z The Z coordinate of the point.
    * 
    * @return The height of the triangle, or -FLT_MAX if the triangle has no heigth (is vertical)
    */
    float GetYOnTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float x, float z);

    /**
    * @brief Checks if two bounding boxes are intersecting.
    * 
    * @param a The first bounding box.
    * @param b The second bounding box.
    * 
    * @return 'true' if the bounding boxes are interescting, 'false' otherwise.
    */
    bool Intersect(const BoundingBox& a, const BoundingBox& b);
};

#endif // PHYSICSHELPER_H_