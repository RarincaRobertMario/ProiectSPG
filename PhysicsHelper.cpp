#include "PhysicsHelper.h"

BoundingBox PhysicsHelper::CalculateAABB(const std::vector<float>& vertices, int stride)
{
    BoundingBox bb;
    for (size_t i = 0; i < vertices.size(); i += stride) 
    {
        glm::vec3 p(vertices[i], vertices[i + 1], vertices[i + 2]);
        bb.min = glm::min(bb.min, p);
        bb.max = glm::max(bb.max, p);
    }
    return bb;
}

float PhysicsHelper::GetYOnTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float x, float z)
{
    // Plane equation: Ax + By + Cz + D = 0
    glm::vec3 edge1 = p2 - p1;
    glm::vec3 edge2 = p3 - p1;
    glm::vec3 normal = glm::cross(edge1, edge2);

    // If the triangle is vertical, it has no height at (x,z)
    if (std::abs(normal.y) < 0.0001f) return -FLT_MAX;

    // D = -(Ax + By + Cz)
    float d = -glm::dot(normal, p1);

    // Solve for y: y = -(Ax + Cz + D) / B
    return -(normal.x * x + normal.z * z + d) / normal.y;
}

bool PhysicsHelper::Intersect(const BoundingBox& a, const BoundingBox& b)
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
        (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
        (a.min.z <= b.max.z && a.max.z >= b.min.z);
}