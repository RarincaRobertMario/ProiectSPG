#pragma once

#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include <glm/glm.hpp>

/**
* @brief Defines a transformation.
*/
struct Transform
{
    glm::vec3 position{ 0.0f, 0.0f, 0.0f }; /// Position of the object (0.0 - center of axis).
    glm::vec3 rotation{ 0.0f, 0.0f, 0.0f }; /// Rotation of the object (radians).
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };    /// Scale of the object (1.0f - default size).

    /**
    * @brief Creates and returns the model matrix of the transformation.
    * 
    * @return The model matrix (4x4) of the transformation, in the order: TRANSLATE * ROTATE * SCALE * I
    */
    glm::mat4 GetModelMatrix() const;
};

#endif // TRANSFORM_H_