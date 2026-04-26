#include "CylinderMesh.h"

CylinderMesh::CylinderMesh(const std::weak_ptr<Shader>& shader, float radius, float height, int sectors, const Transform& transform, const glm::vec3& color) :
    Mesh(shader)
{
    this->transform = transform;
    this->meshColor = color;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    GenerateCylinder(radius, height, sectors, vertices, indices, color);

    SetupMesh(vertices.data(), vertices.size(), indices.data(), indices.size());
}

float* CylinderMesh::GenerateCylinder(float radius, float height, int sectors, std::vector<float>& outV, std::vector<unsigned int>& outI, const glm::vec3& color)
{
    outV.clear();
    outI.clear();
    float halfH = height / 2.0f;
    constexpr float PI = 3.14159265359f;

    // Pos(3), Norm(3), UV(2), Tangent(3), Color(3)

    // Side verticles
    for (int i = 0; i <= sectors; ++i) 
    {
        float segment = (float)i / (float)sectors;
        float angle = 2.0f * PI * segment;
        float x = cos(angle);
        float z = sin(angle);

        float nx = x; 
        float nz = z;
        float tx = -sin(angle); 
        float tz = cos(angle);

        // Bottom vertex
        outV.insert(outV.end(), { x * radius, -halfH, z * radius, nx, 0.0f, nz, segment, 0.0f, tx, 0.0f, tz, color.r, color.g, color.b });
        // Top vertex
        outV.insert(outV.end(), { x * radius,  halfH, z * radius, nx, 0.0f, nz, segment, 1.0f, tx, 0.0f, tz, color.r, color.g, color.b });
    }

    // Indices for the side verticels
    for (int i = 0; i < sectors; ++i) 
    {
        unsigned int b0 = i * 2, t0 = i * 2 + 1, b1 = (i + 1) * 2, t1 = (i + 1) * 2 + 1;
        outI.insert(outI.end(), { b0, b1, t1, b0, t1, t0 });
    }

    // Bottom circle
    unsigned int bottomCenterIndex = static_cast<unsigned int>(outV.size()) / STRIDE;

    outV.insert(outV.end(), { 0.0f, -halfH, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, color.r, color.g, color.b });
    for (int i = 0; i <= sectors; ++i) 
    {
        float angle = 2.0f * PI * (float)i / (float)sectors;
        float x = cos(angle), z = sin(angle);
        float u = x * 0.5f + 0.5f; 
        float v = z * 0.5f + 0.5f;

        outV.insert(outV.end(), { x * radius, -halfH, z * radius, 0.0f, -1.0f, 0.0f, u, v, 1.0f, 0.0f, 0.0f, color.r, color.g, color.b });
        if (i < sectors) 
        {
            // Indices
            unsigned int start = bottomCenterIndex + 1;
            outI.insert(outI.end(), { bottomCenterIndex, start + i + 1, start + i });
        }
    }

    // Top circle
    unsigned int topCenterIndex = static_cast<unsigned int>(outV.size()) / STRIDE;

    outV.insert(outV.end(), { 0.0f, halfH, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, color.r, color.g, color.b });
    for (int i = 0; i <= sectors; ++i) 
    {
        float angle = 2.0f * PI * (float)i / (float)sectors;
        float x = cos(angle), z = sin(angle);
        float u = x * 0.5f + 0.5f; 
        float v = z * 0.5f + 0.5f;

        outV.insert(outV.end(), { x * radius, halfH, z * radius, 0.0f, 1.0f, 0.0f, u, v, 1.0f, 0.0f, 0.0f, color.r, color.g, color.b });
        if (i < sectors) 
        {
            // Indices
            unsigned int start = topCenterIndex + 1;
            outI.insert(outI.end(), { topCenterIndex, start + i, start + i + 1 });
        }
    }

    return outV.data();
}