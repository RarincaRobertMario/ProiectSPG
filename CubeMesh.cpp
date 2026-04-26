#include "CubeMesh.h"

CubeMesh::CubeMesh(const std::weak_ptr<Shader>& shader, const Transform& transform, const glm::vec3& color) :
	Mesh(cubeVertices, sizeof(cubeVertices) / sizeof(float), shader, transform, color, cubeIndices, sizeof(cubeIndices) / sizeof(unsigned int)) {}