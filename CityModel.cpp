#include <glad/glad.h>
#include <iostream>
#include <glm/ext/matrix_transform.hpp>
#include <random>
#include <map>

#include "CityModel.h"

#include "CubeMesh.h"
#include "MeshCache.h"

CityModel::CityModel(const Transform& t) :
    transform(t)
{
    bakedCity = std::make_unique<CityMeshData>();
}

void CityModel::Draw(std::shared_ptr<Shader> customShader) const
{
    glm::mat4 cityMatrix = transform.GetModelMatrix();
    auto activeShader = customShader ? customShader : bakedCity->bakedBodyMesh->GetShader().lock();

    if (!activeShader)
    {
        std::cerr << "{PoliceCarModel::Draw} ERROR: Custom shader is NULLPTR and self-shader is NULLPTR or not set." << std::endl;
        return;
    }

    activeShader->UseShader();

    glUniformMatrix4fv(activeShader->GetUniformLocation("modelMatrix"), 1, GL_FALSE, &cityMatrix[0][0]);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(cityMatrix)));
    glUniformMatrix3fv(activeShader->GetUniformLocation("normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

    glUniform1i(activeShader->GetUniformLocation("isLightMesh"), 0);
    
    bakedCity->bakedBodyMesh->DrawRaw(customShader);

    if (bakedCity->bakedAlphaMesh)
    {
        glEnable(GL_BLEND);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);

        bakedCity->bakedAlphaMesh->DrawRaw(customShader);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    if (bakedCity->bakedLightMesh)
    {
        glUniform1i(activeShader->GetUniformLocation("isLightMesh"), 1);
        glUniform1f(activeShader->GetUniformLocation("lightMeshIntensity"), 1.0f);
        bakedCity->bakedLightMesh->DrawRaw(customShader);
        glUniform1i(activeShader->GetUniformLocation("isLightMesh"), 0);
    }
}

std::vector<std::vector<CityTileType>> CityModel::GenerateCity(const std::weak_ptr<Shader>& shader, int size)
{
    std::vector<std::vector<CityTileType>> matrix(size, std::vector<CityTileType>(size, CityTileType::Skyscraper));

    /**
    * @brief Structure for a node.
    */
    struct Node 
    { 
        int x;  /// X coordinate.
        int z;  /// Z coordinate.
    };

    // Grid.
    std::vector<std::vector<Node>> grid;

    // Spacing (how dense or sparse roads are).
    int spacing = (size <= 12) ? 4 : 4;

    // Creating graph nodes (intersections)
    for (int z = 1; z < size; z += spacing) 
    {
        std::vector<Node> row;
        for (int x = 1; x < size; x += spacing) 
        {
            // ox and oz are random 1 or 0, which shifts the intersections in a random direction, in order to not
            // have straight lines
            int ox = (rand() % 2);
            int oz = (rand() % 2);
            row.push_back({ std::clamp(x + ox, 0, size - 1), std::clamp(z + oz, 0, size - 1) });
        }
        grid.push_back(row);
    }

    // Function to draw road path between two nodes (intersections) (ray-tracing)
    auto DrawSimplePath = [&](Node n1, Node n2) {
        int cx = n1.x, cz = n1.z;
        // Move until the X matches
        while (cx != n2.x) 
        { 
            // Intersection here is used as a 'placeholder' tile
            matrix[cz][cx] = CityTileType::Intersection; 
            cx += (n2.x > cx) ? 1 : -1; 
        }

        // Move until the Z matches
        while (cz != n2.z) 
        { 
            matrix[cz][cx] = CityTileType::Intersection; 
            cz += (n2.z > cz) ? 1 : -1; 
        }
        matrix[cz][cx] = CityTileType::Intersection;
    };

    // Get each node in the grid and draw pathes between each other
    for (size_t r = 0; r < grid.size(); ++r) 
    {
        for (size_t c = 0; c < grid[r].size(); ++c) 
        {
            if (c + 1 < grid[r].size()) 
            {
                DrawSimplePath(grid[r][c], grid[r][c + 1]);
            }
            if (r + 1 < grid.size())    
            {
                DrawSimplePath(grid[r][c], grid[r + 1][c]);
            }
        }
    }

    // Parks!
    for (int z = 0; z < size; ++z) 
    {
        for (int x = 0; x < size; ++x) 
        {
            if (matrix[z][x] == CityTileType::Skyscraper) 
            {
                // 15% chance 
                if ((rand() % 100) < 15) 
                {
                    matrix[z][x] = CityTileType::Park;
                }
            }
        }
    }

    // Auto-tilling
    auto finalMatrix = matrix;

    // Checks if a tile is a road
    auto IsRoad = [&](int r, int c) {
        if (r < 0 || r >= size || c < 0 || c >= size)
        {
            return false;
        }
        CityTileType t = matrix[r][c];
        return t != CityTileType::Skyscraper && t != CityTileType::Park;
    };

    // For each tile
    for (int z = 0; z < size; ++z) 
    {
        for (int x = 0; x < size; ++x) 
        {
            // Skip non-road tiles
            if (matrix[z][x] == CityTileType::Skyscraper || matrix[z][x] == CityTileType::Park)
            {
                continue;
            }

            bool N = IsRoad(z - 1, x), S = IsRoad(z + 1, x),
                E = IsRoad(z, x + 1), W = IsRoad(z, x - 1);
            int conn = static_cast<int>(N) + static_cast<int>(S) + static_cast<int>(E) + static_cast<int>(W);

            if (conn >= 3)    finalMatrix[z][x] = CityTileType::Intersection;
            else if (N && S)  finalMatrix[z][x] = CityTileType::StreetVertical;
            else if (E && W)  finalMatrix[z][x] = CityTileType::StreetHorizontal;
            else if (S && E)  finalMatrix[z][x] = CityTileType::CornerNE;
            else if (S && W)  finalMatrix[z][x] = CityTileType::CornerNW;
            else if (N && E)  finalMatrix[z][x] = CityTileType::CornerSE;
            else if (N && W)  finalMatrix[z][x] = CityTileType::CornerSW;
            else              finalMatrix[z][x] = CityTileType::Intersection;
        }
    }

    BuildModelFromMatrix(shader, finalMatrix);
    Bake(shader);
    return finalMatrix;
}

void CityModel::BuildModelFromMatrix(const std::weak_ptr<Shader>& shader, const std::vector<std::vector<CityTileType>>& matrix)
{
    float tileSize = 16.0f;
    int size = static_cast<int>(matrix.size());
    float offset = (size / 2.0f) * tileSize;

    for (int z = 0; z < size; ++z) 
    {
        for (int x = 0; x < size; ++x) 
        {
            // Convert grid index to world position
            // (x=0, z=0) becomes (-offset, 0, -offset)
            glm::vec3 worldPos(x * tileSize - offset, 0, z * tileSize - offset);

            switch (matrix[z][x]) 
            {
                case CityTileType::Park:             CreateParkTile(shader, worldPos); break;
                case CityTileType::Skyscraper:       CreateSkyscraperTile(shader, worldPos); break;
                case CityTileType::StreetVertical:   CreateStreetTileVertical(shader, worldPos); break;
                case CityTileType::StreetHorizontal: CreateStreetTileHorizontal(shader, worldPos); break;
                case CityTileType::Intersection:     CreateIntersectionTile(shader, worldPos); break;
                case CityTileType::CornerNE:         CreateCornerNE(shader, worldPos); break;
                case CityTileType::CornerNW:         CreateCornerNW(shader, worldPos); break;
                case CityTileType::CornerSE:         CreateCornerSE(shader, worldPos); break;
                case CityTileType::CornerSW:         CreateCornerSW(shader, worldPos); break;
                default: CreateParkTile(shader, worldPos); break;
            }
        }
    }
}

void CityModel::CreateCornerNE(const std::weak_ptr<Shader>& shader, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto AddPartAligned = [&](glm::vec3 lPos, glm::vec3 lScale, glm::vec3 color) {
        lPos.y += (lScale.y * 0.5f);
        AddPart(shader, BuildingPartType::Foundation, color, pos + lPos, lScale, glm::vec3(0));
    };

    // Asphalt
    AddPartAligned(glm::vec3(1.5f, 0, 1.5f), glm::vec3(13, 0.15f, 13), glm::vec3(0.08f));

    // Sidewalks
    AddPartAligned(glm::vec3(0, 0, -6.5f), glm::vec3(16, 0.3f, 3), glm::vec3(0.35f));  // South strip
    AddPartAligned(glm::vec3(-6.5f, 0, 1.5f), glm::vec3(3, 0.3f, 13), glm::vec3(0.35f)); // West strip
    AddPartAligned(glm::vec3(6.5f, 0, 6.5f), glm::vec3(3, 0.3f, 3), glm::vec3(0.35f)); // NE Square
}

void CityModel::CreateCornerNW(const std::weak_ptr<Shader>& shader, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto AddPartAligned = [&](glm::vec3 lPos, glm::vec3 lScale, glm::vec3 color) {
        lPos.y += (lScale.y * 0.5f);
        AddPart(shader, BuildingPartType::Foundation, color, pos + lPos, lScale, glm::vec3(0));
    };

    AddPartAligned(glm::vec3(-1.5f, 0, 1.5f), glm::vec3(13, 0.15f, 13), glm::vec3(0.08f));

    AddPartAligned(glm::vec3(0, 0, -6.5f), glm::vec3(16, 0.3f, 3), glm::vec3(0.35f));  // South strip
    AddPartAligned(glm::vec3(6.5f, 0, 1.5f), glm::vec3(3, 0.3f, 13), glm::vec3(0.35f));  // East strip
    AddPartAligned(glm::vec3(-6.5f, 0, 6.5f), glm::vec3(3, 0.3f, 3), glm::vec3(0.35f)); // NW Square
}

void CityModel::CreateCornerSE(const std::weak_ptr<Shader>& shader, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto AddPartAligned = [&](glm::vec3 lPos, glm::vec3 lScale, glm::vec3 color) {
        lPos.y += (lScale.y * 0.5f);
        AddPart(shader, BuildingPartType::Foundation, color, pos + lPos, lScale, glm::vec3(0));
    };

    AddPartAligned(glm::vec3(1.5f, 0, -1.5f), glm::vec3(13, 0.15f, 13), glm::vec3(0.08f));

    AddPartAligned(glm::vec3(0, 0, 6.5f), glm::vec3(16, 0.3f, 3), glm::vec3(0.35f));   // North strip
    AddPartAligned(glm::vec3(-6.5f, 0, -1.5f), glm::vec3(3, 0.3f, 13), glm::vec3(0.35f)); // West strip
    AddPartAligned(glm::vec3(6.5f, 0, -6.5f), glm::vec3(3, 0.3f, 3), glm::vec3(0.35f)); // SE Square
}

void CityModel::CreateCornerSW(const std::weak_ptr<Shader>& shader, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto AddPartAligned = [&](glm::vec3 lPos, glm::vec3 lScale, glm::vec3 color) {
        lPos.y += (lScale.y * 0.5f);
        AddPart(shader, BuildingPartType::Foundation, color, pos + lPos, lScale, glm::vec3(0));
    };

    AddPartAligned(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(13, 0.15f, 13), glm::vec3(0.08f));

    AddPartAligned(glm::vec3(0, 0, 6.5f), glm::vec3(16, 0.3f, 3), glm::vec3(0.35f));  // North strip
    AddPartAligned(glm::vec3(6.5f, 0, -1.5f), glm::vec3(3, 0.3f, 13), glm::vec3(0.35f)); // East strip
    AddPartAligned(glm::vec3(-6.5f, 0, -6.5f), glm::vec3(3, 0.3f, 3), glm::vec3(0.35f)); // SW Square
}

void CityModel::CreateStreetTileVertical(const std::weak_ptr<Shader>&shader, const glm::vec3 & pos, const glm::vec3 & scale, const glm::vec3 & rot)
{
    constexpr float tileSize = 16.0f;
    auto AddPartAligned = [&](glm::vec3 lPos, glm::vec3 lScale, glm::vec3 color) {
        lPos.y += (lScale.y * 0.5f);
        AddPart(shader, BuildingPartType::Foundation, color, pos + lPos, lScale, glm::vec3(0));
    };

    AddPartAligned(glm::vec3(0, 0, 0), glm::vec3(10, 0.15f, 16), glm::vec3(0.08f)); // Asphalt
    for (float z = -6.0f; z <= 6.0f; z += 4.0f)
    {
        AddPartAligned(glm::vec3(0, 0.16f, z), glm::vec3(0.2f, 0.02f, 2.0f), glm::vec3(0.9f)); // Stripes
    }

    AddPartAligned(glm::vec3(-6.5f, 0, 0), glm::vec3(3, 0.3f, 16), glm::vec3(0.35f)); // West Sidewalk
    AddPartAligned(glm::vec3(6.5f, 0, 0), glm::vec3(3, 0.3f, 16), glm::vec3(0.35f)); // East Sidewalk
}

void CityModel::CreateStreetTileHorizontal(const std::weak_ptr<Shader>& shader, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    constexpr float tileSize = 16.0f;
    auto AddPartAligned = [&](glm::vec3 lPos, glm::vec3 lScale, glm::vec3 color) {
        lPos.y += (lScale.y * 0.5f);
        AddPart(shader, BuildingPartType::Foundation, color, pos + lPos, lScale, glm::vec3(0));
    };

    AddPartAligned(glm::vec3(0, 0, 0), glm::vec3(16, 0.15f, 10), glm::vec3(0.08f)); // Asphalt
    for (float x = -6.0f; x <= 6.0f; x += 4.0f)
    {
        AddPartAligned(glm::vec3(x, 0.16f, 0), glm::vec3(2.0f, 0.02f, 0.2f), glm::vec3(0.9f)); // Stripes
    }

    AddPartAligned(glm::vec3(0, 0, 6.5f), glm::vec3(16, 0.3f, 3), glm::vec3(0.35f)); // North Sidewalk
    AddPartAligned(glm::vec3(0, 0, -6.5f), glm::vec3(16, 0.3f, 3), glm::vec3(0.35f)); // South Sidewalk
}

void CityModel::CreateIntersectionTile(const std::weak_ptr<Shader>& shader, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto AddPartAligned = [&](glm::vec3 lPos, glm::vec3 lScale, glm::vec3 color) {
        lPos.y += (lScale.y * 0.5f);
        AddPart(shader, BuildingPartType::Foundation, color, pos + lPos, lScale, glm::vec3(0));
    };

    AddPartAligned(glm::vec3(0, 0, 0), glm::vec3(16, 0.15f, 16), glm::vec3(0.08f));

    // Corner Sidewalk Squares
    float cornerPos = 6.5f;
    AddPartAligned(glm::vec3(-cornerPos, 0, -cornerPos), glm::vec3(3, 0.3f, 3), glm::vec3(0.35f)); // SW
    AddPartAligned(glm::vec3(cornerPos, 0, -cornerPos), glm::vec3(3, 0.3f, 3), glm::vec3(0.35f)); // SE
    AddPartAligned(glm::vec3(-cornerPos, 0, cornerPos), glm::vec3(3, 0.3f, 3), glm::vec3(0.35f)); // NW
    AddPartAligned(glm::vec3(cornerPos, 0, cornerPos), glm::vec3(3, 0.3f, 3), glm::vec3(0.35f)); // NE
}

void CityModel::CreateSkyscraperTile(const std::weak_ptr<Shader>& shader, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    float towerSize = 12.0f;
    int floors = 6;
    float floorH = 3.0f;
    float foundationH = 0.4f;

    // Foundation
    AddPart(shader, BuildingPartType::Foundation, glm::vec3(0.3f), pos + glm::vec3(0, 0.2f, 0), glm::vec3(16.0f, 0.4f, 16.0f), glm::vec3(0));
    
    // Main tower
    for (int i = 0; i < floors; ++i)
    {
        float yPos = foundationH + (i * floorH) + (floorH * 0.5f);
        glm::vec3 bodyColor = (i % 2 == 0) ? glm::vec3(0.15f) : glm::vec3(0.18f);

        // Core Structure
        AddPart(shader, BuildingPartType::Wall, bodyColor, pos + glm::vec3(0, yPos, 0), glm::vec3(towerSize, floorH, towerSize), glm::vec3(0));

        // Windows
        glm::vec3 glassColor = glm::vec3(0.1f, 0.3f, 0.5f);
        float windowHeight = floorH * 0.6f;
        
        // North/South Glass
        AddPart(shader, BuildingPartType::Wall, glassColor, pos + glm::vec3(0, yPos, 6.01f), glm::vec3(10.0f, windowHeight, 0.1f), glm::vec3(0));
        AddPart(shader, BuildingPartType::Wall, glassColor, pos + glm::vec3(0, yPos, -6.01f), glm::vec3(10.0f, windowHeight, 0.1f), glm::vec3(0));
        // East/West Glass
        AddPart(shader, BuildingPartType::Wall, glassColor, pos + glm::vec3(6.01f, yPos, 0), glm::vec3(0.1f, windowHeight, 10.0f), glm::vec3(0));
        AddPart(shader, BuildingPartType::Wall, glassColor, pos + glm::vec3(-6.01f, yPos, 0), glm::vec3(0.1f, windowHeight, 10.0f), glm::vec3(0));
    }
}

void CityModel::CreateParkTile(const std::weak_ptr<Shader>& shader, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto AddTilePart = [&](glm::vec3 lPos, glm::vec3 lSize, glm::vec3 color) {
        lPos.y += (lSize.y * 0.5f);
        AddPart(shader, BuildingPartType::Foundation, color, pos + lPos, lSize, glm::vec3(0));
    };

    // Grass Base
    AddTilePart(glm::vec3(0, 0, 0), glm::vec3(16, 0.2f, 16), glm::vec3(0.1f, 0.4f, 0.1f));

    // Scattered Trees
    glm::vec3 trees[] = { {5,0,5}, {-4,0,-6}, {6,0,-4} };
    for (auto t : trees) 
    {
        AddTilePart(t + glm::vec3(0, 0.2f, 0), glm::vec3(0.4f, 2.0f, 0.4f), glm::vec3(0.25f, 0.15f, 0.05f)); // Trunk
        AddTilePart(t + glm::vec3(0, 2.0f, 0), glm::vec3(3.0f, 2.5f, 3.0f), glm::vec3(0.0f, 0.5f, 0.0f));  // Leaves
    }
}

void CityModel::AddPart(const std::weak_ptr<Shader>& shader, BuildingPartType type, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto cube = std::make_unique<CubeMesh>(shader, Transform{ pos, rot, scale }, color);

    parts.push_back({ std::move(cube), BuildingMeshType::Solid, type });
}

void CityModel::AddWindow(const std::weak_ptr<Shader>& shader, uint8_t alpha, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    if (!bakedCity->windowTex)
    {
        bakedCity->windowTex = Texture::CreateSolidColor(
            static_cast<unsigned char>(color.r * 255),
            static_cast<unsigned char>(color.g * 255),
            static_cast<unsigned char>(color.b * 255),
            alpha,
            "texture_diffuse"
        );
    }

    auto mesh = std::make_unique<CubeMesh>(shader, Transform{ pos, rot, scale }, color);
    parts.push_back({ std::move(mesh), BuildingMeshType::Alpha, BuildingPartType::Window });
}

void CityModel::AddLight(const std::weak_ptr<Shader>& shader, const glm::vec3& color, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot)
{
    auto bulb = std::make_unique<CubeMesh>(shader, Transform{ pos, rot, scale }, color);
    parts.push_back({ std::move(bulb), BuildingMeshType::Light, BuildingPartType::Light });
}

void CityModel::Bake(const std::weak_ptr<Shader>& shader)
{
    bakedCity->bakedBodyMesh = PerformBakePass(shader, BakePassType::Solid);
    bakedCity->bakedAlphaMesh = PerformBakePass(shader, BakePassType::Alpha);
    bakedCity->bakedLightMesh =  PerformBakePass(shader, BakePassType::Light);
    parts.clear();

    if (bakedCity->bakedAlphaMesh && bakedCity->windowTex)
    {
        bakedCity->bakedAlphaMesh->SetTexture(bakedCity->windowTex);
    }
}

std::unique_ptr<Mesh> CityModel::PerformBakePass(const std::weak_ptr<Shader>& shader, BakePassType passType)
{
    std::vector<float> allVertices;
    std::vector<unsigned int> allIndices;

    std::vector<MeshCache> batches;
    batches.reserve(parts.size());

    size_t totalVertFloats = 0;
    size_t totalIndices = 0;

    for (size_t i = 0; i < parts.size(); ++i)
    {
        auto& part = parts[i];
        if (!part.mesh)
        {
            continue;
        }

        if (passType == BakePassType::Solid && part.meshType != BuildingMeshType::Solid) continue;
        if (passType == BakePassType::Alpha && part.meshType != BuildingMeshType::Alpha) continue;
        if (passType == BakePassType::Light && part.meshType != BuildingMeshType::Light) continue;

        auto& mesh = part.mesh;

        MeshCache batch;
        batch.verts = mesh->GetVertexData();
        batch.inds = mesh->GetIndexData();
        batch.transform = mesh->GetTransform().GetModelMatrix();

        totalVertFloats += batch.verts.size();
        totalIndices += batch.inds.size();

        batches.push_back(std::move(batch));
    }

    allVertices.reserve(totalVertFloats);
    allIndices.reserve(totalIndices);

    for (const auto& batch : batches)
    {
        // vertOffset is the count of vertices currently in the buffer
        const unsigned int vertOffset = static_cast<unsigned int>(allVertices.size() / STRIDE);

        glm::mat3 normMat = glm::mat3(glm::transpose(glm::inverse(batch.transform)));

        for (size_t v = 0; v < batch.verts.size(); v += STRIDE)
        {
            // Transform Position
            glm::vec4 pos = batch.transform * glm::vec4(batch.verts[v], batch.verts[v + 1], batch.verts[v + 2], 1.0f);
            allVertices.push_back(pos.x);
            allVertices.push_back(pos.y);
            allVertices.push_back(pos.z);

            // Transform Normal
            glm::vec3 norm = glm::normalize(normMat * glm::vec3(batch.verts[v + 3], batch.verts[v + 4], batch.verts[v + 5]));
            allVertices.push_back(norm.x);
            allVertices.push_back(norm.y);
            allVertices.push_back(norm.z);

            // Bulk copy UVs, Tangents, and Colors
            allVertices.insert(allVertices.end(),
                batch.verts.begin() + v + 6,
                batch.verts.begin() + v + 14
            );
        }

        // Offset indices so they point to the correct vertices in the new global buffer
        for (auto idx : batch.inds)
        {
            allIndices.push_back(idx + vertOffset);
        }
    }

    if (allVertices.empty())
    {
        return nullptr;
    }

    return std::make_unique<Mesh>(
        allVertices.data(),
        allVertices.size(),
        shader,
        Transform{},
        glm::vec3(1.0f),
        allIndices.data(),
        allIndices.size()
    );
}
