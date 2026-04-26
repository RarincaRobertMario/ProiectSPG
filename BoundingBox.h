#pragma once

#ifndef BOUNDINGBOX_H_
#define BOUNDINGBOX_H_

#include <glm/glm.hpp>

/**
* @brief Structure for a bounding box.
*/
struct BoundingBox 
{
    glm::vec3 min = glm::vec3(FLT_MAX);
    glm::vec3 max = glm::vec3(-FLT_MAX);
};

#endif // BOUNDINGBOX_H_