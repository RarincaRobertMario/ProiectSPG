#pragma once

#ifndef CYLINDERMESH_H_
#define CYLINDERMESH_H_

#include <vector>
#include <cmath>
#include <glm/glm.hpp>

#include "Mesh.h"

/**
* @brief A cylinder mesh.
*/
class CylinderMesh : public Mesh 
{
private:

    /**
    * @brief Generates a cylinder.
    * 
    * @param r Radius of the cylinder.
    * @param h Height of the cylinder.
    * @param s 'Detail' (sectors) of cylinder.
    * @param [out] outV Vector of vertices.
    * @param [out] outI Vector of indeces.
    * @param color Color of the cylinder.
    * 
    * @return The raw vertices data.
    */
    static float* GenerateCylinder(
        float radius,
        float height,
        int sectors,
        std::vector<float>& outV,
        std::vector<unsigned int>& outI,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f)
    );

public:

    /**
    * @brief Constructor.
    * 
    * @param shader Weak pointer to shader.
    * @param radius Radius of the cylinder.
    * @param height Height of the cylinder
    * @param sectors Sectors ('detail')  of the cylinder
    * @param transform Optional transformation.
    * @param color Color of the cylinder
    */
    CylinderMesh(
        const std::weak_ptr<Shader>& shader,
        float radius = 0.5f, 
        float height = 1.0f, 
        int sectors = 16, 
        const Transform& transform = {}, 
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f)
    );

    /**
    * @brief Default destructor.
    */
    ~CylinderMesh() override = default;
};

#endif // CYLINDERMESH_H_