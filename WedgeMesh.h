#pragma once

#ifndef WEDGEMESH_H_
#define WEDGEMESH_H_

#include "Mesh.h"

/**
* @brief A 'right angle' wedge mesh. Think of a square that has been cut in two right triangles.
*/
class WedgeMesh : public Mesh
{
private:

    static inline float wedgeVertices[] = {
        // BACK FACE
        // Position           // Normal            // UV        // Tangents
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,

        // BOTTOM FACE
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,  1.0f, 0.0f, 0.0f,

        // SLOPE
        -0.5f, -0.5f,  0.5f,  0.0f,  0.707f, 0.707f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.707f, 0.707f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.707f, 0.707f, 1.0f, 1.0f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.707f, 0.707f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f,

        // LEFT TRIANGLE
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.5f, 1.0f,  0.0f, 0.0f, 1.0f,

        // RIGHT TRIANGLE
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,  0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.5f, 1.0f,  0.0f, 0.0f, -1.0f
    };

    static inline unsigned int wedgeIndices[] = {
        0, 1, 2, 0, 2, 3,       // Back
        4, 5, 6, 4, 6, 7,       // Bottom
        8, 9, 10, 8, 10, 11,    // Slope
        12, 13, 14,             // Left Triangle
        15, 16, 17              // Right Triangle
    };

public:

    /**
    * @brief Constructor
    *
    * @param shader Weak pointer to the shader used
    * @param transform Optional transformation
    * @param color Color of the wedge
    */
    WedgeMesh(
        const std::weak_ptr<Shader>& shader,
        const Transform& transform = {},
        const glm::vec3& color = { 1.0f, 1.0f, 1.0f }
    );

    /**
    * @brief Default destructor.
    */
    ~WedgeMesh() override = default;
};

#endif // WEDGEMESH_H_