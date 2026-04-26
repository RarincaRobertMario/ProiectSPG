#include "WedgeMesh.h"

WedgeMesh::WedgeMesh(const std::weak_ptr<Shader>& shader, const Transform& transform, const glm::vec3& color) :
	Mesh(wedgeVertices, sizeof(wedgeVertices) / sizeof(float), shader, transform, color, wedgeIndices, sizeof(wedgeIndices) / sizeof(unsigned int)) {}

