#pragma once

#ifndef CITYMODEL_H_
#define CITYMODEL_H_

#include <glm/glm.hpp>
#include <random>

#include "IRenderable.h"
#include "CityMeshData.h"
#include "CityPart.h"
#include "BuildingPartType.h"
#include "BakePassType.h"
#include "CityTileType.h"

constexpr int CITY_SIZE = 25;

class CityModel : public IRenderable
{
private:
    std::vector<CityPart> parts;                /// Vector of parts.
    std::shared_ptr<CityMeshData> bakedCity;    /// Baked city mesh.
    Transform transform;    /// Global transform.

    /**
    * @brief Builds the model from a matrix.
    * 
    * @param shader Weak pointer to the shader.
    * @param matrix Matrix of tiles.
    */
    void BuildModelFromMatrix(
        const std::weak_ptr<Shader>& shader,
        const std::vector<std::vector<CityTileType>>& matrix
    );

    /**
    * @brief Creates a NE corner.
    * 
    * @param shader Weak pointer to the shader.
    * @param pos Position of the corner.
    * @param scale Scale of the corner.
    * @param rot Rotation of the corner.
    */
    void CreateCornerNE(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates a NW corner.
    *
    * @param shader Weak pointer to the shader.
    * @param pos Position of the corner.
    * @param scale Scale of the corner.
    * @param rot Rotation of the corner.
    */
    void CreateCornerNW(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates a SE corner.
    *
    * @param shader Weak pointer to the shader.
    * @param pos Position of the corner.
    * @param scale Scale of the corner.
    * @param rot Rotation of the corner.
    */
    void CreateCornerSE(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates a SW corner.
    *
    * @param shader Weak pointer to the shader.
    * @param pos Position of the corner.
    * @param scale Scale of the corner.
    * @param rot Rotation of the corner.
    */
    void CreateCornerSW(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );


    /**
    * @brief Creates a street tile vertical (-Z <-> +Z)
    * 
    * @param shader Weak pointer to the shader.
    * @param pos Position of the street.
    * @param scale Scale of the street.
    * @param rot Rotation of the street.
    */
    void CreateStreetTileVertical(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates a street tile horizontal (-X <-> +X)
    *
    * @param shader Weak pointer to the shader.
    * @param pos Position of the street.
    * @param scale Scale of the street.
    * @param rot Rotation of the street.
    */
    void CreateStreetTileHorizontal(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates a 4-way intersection tile
    *
    * @param shader Weak pointer to the shader.
    * @param pos Position of the intersection.
    * @param scale Scale of the intersection.
    * @param rot Rotation of the intersection.
    */
    void CreateIntersectionTile(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates a skyscraper
    *
    * @param shader Weak pointer to the shader.
    * @param pos Position of the skyscraper.
    * @param scale Scale of the skyscraper.
    * @param rot Rotation of the skyscraper.
    */
    void CreateSkyscraperTile(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates a park
    *
    * @param shader Weak pointer to the shader.
    * @param pos Position of the park.
    * @param scale Scale of the park.
    * @param rot Rotation of the park.
    */
    void CreateParkTile(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a part.
    *
    * @param shader Weak pointer to the shader.
    * @param type The building part type.
    * @param color Color of the part
    * @param pos Position of the part
    * @param scale Scale of the part
    * @param rot Rotation of the part
    */
    void AddPart(
        const std::weak_ptr<Shader>& shader,
        BuildingPartType type,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a window part.
    *
    * @param shader Weak pointer to the shader.
    * @param alpha Tranparency of the window.
    * @param color Color of the part
    * @param pos Position of the part
    * @param scale Scale of the part
    * @param rot Rotation of the part
    */
    void AddWindow(
        const std::weak_ptr<Shader>& shader,
        uint8_t alpha = 255,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a light part.
    *
    * NOTE THAT IT DOES NOT AUTOMATICALLY MAKE IT HAVE AN ACTUAL LIGHT, JUST MAKES THE MESH GLOW AND BE BRIGHT
    *
    * @param shader Weak pointer to the shader.
    * @param color Color of the part
    * @param pos Position of the part
    * @param scale Scale of the part
    * @param rot Rotation of the part
    */
    void AddLight(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Bakes the city mesh.
    * 
    * @param shader Weak pointer to the shader.
    */
    void Bake(const std::weak_ptr<Shader>& shader);

    /**
    * @brief Performs a bake pass.
    * 
    * @param shader Weak pointer to the shader.
    * @param passType The type of bake pass.
    * 
    * @return Pointer to the baked mesh.
    */
    std::unique_ptr<Mesh> PerformBakePass(const std::weak_ptr<Shader>& shader, BakePassType passType);

public:

    /**
    * @brief Constructor.
    * 
    * @param t Optional global transform
    */
    CityModel(const Transform& t = {});

    /**
    * @brief Generates a city.
    *
    * @param shader Weak pointer to the shader.
    * @param size Size of the city (the city is a square).
    *
    * @return A matrix of tile types.
    */
    std::vector<std::vector<CityTileType>> GenerateCity(const std::weak_ptr<Shader>& shader, int size);

    /**
    * @brief Draws the city.
    * 
    * @param customeShader Custom shader. If nullptr, it will use the meshes' own shader.
    */
    void Draw(std::shared_ptr<Shader> customShader = nullptr) const override;
};

#endif // CITYMODEL_H_