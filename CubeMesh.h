#pragma once

#ifndef CUBEMESH_H_
#define CUBEMESH_H_

struct vec3;

#include "Mesh.h"

/**
* @brief A cube mesh.
*/
class CubeMesh : public Mesh 
{
private:

    // Format X Y Z | NX NY NZ | U V | TX TY TZ
    static inline float cubeVertices[] = {
        // Back face
        // Pos                  // Normal          // UV          // Tangent
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,

        // Front face
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,

        // Left face
        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   0.0f, 0.0f, 1.0f,

        // Right face
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 0.0f,   0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f, -1.0f,
         0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 1.0f,   0.0f, 0.0f, -1.0f,
         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 0.0f,   0.0f, 0.0f, -1.0f,

         // Top face
         -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
          0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
          0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f,

         // Bottom face
         -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
          0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,
          0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f,   1.0f, 0.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f, 0.0f
    };

    static inline unsigned int cubeIndices[] = {
        0,1,2,    0,2,3,        // Back
        4,5,6,    4,6,7,        // Front
        8,9,10,   8,10,11,      // Left
        12,13,14, 12,14,15,     // Right
        16,17,18, 16,18,19,     // Top
        20,21,22, 20,22,23      // Bottom
    };

public:

    /**
    * @brief Constructor
    * 
    * @param shader Weak pointer to the shader used
    * @param transform Optional transformation
    * @param color Color of the cube
    */
    CubeMesh(
        const std::weak_ptr<Shader>& shader, 
        const Transform& transform = {}, 
        const glm::vec3 & color = { 1.0f, 1.0f, 1.0f }
    );

    /**
    * @brief Default destructor.
    */
    ~CubeMesh() override = default;
};

#endif // CUBEMESH_H_
