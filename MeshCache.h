#pragma once

#ifndef MESHCACHE_H_
#define MESHCACHE_H_

#include <vector>
#include <glm/glm.hpp>

/**
* @brief Structure for a mesh cache. Used for baking.
*/
struct MeshCache
{
    std::vector<float> verts;               /// Vertices of the mesh.
    std::vector<unsigned int> inds;         /// Indices of the mesh.
    glm::mat4 transform = glm::mat4(1.0f);  /// Transform matrix of the mesh.
};

#endif // MESHCACHE_H_