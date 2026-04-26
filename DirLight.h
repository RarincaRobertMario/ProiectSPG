#pragma once

#ifndef DIRLIGHT_H_
#define DIRLIGHT_H_

#include <glm/glm.hpp>
#include <string>

class Shader;

/**
* @brief Directional light of infinite direction.
*/
struct DirLight
{
	std::string name;	/// Name of the directional light.
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);	/// Direction of the light.
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);		/// Color of the light.
	float intensity = 1.0f;	/// Intesity of the light.

    /**
	* @brief Applies the light on a shader.
	*
	* @param shader Reference to the shader.
	*/
    void Apply(const Shader& shader) const;
};

#endif // DIRLIGHT_H_