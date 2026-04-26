#pragma once

#ifndef CIVILIANCARACTIVE_H_
#define CIVILIANCARACTIVE_H_

#include <glm/glm.hpp>
#include <array>

#include "IRenderable.h"
#include "IUpdateable.h"
#include "Mesh.h"
#include "CivilianCarPart.h"
#include "FacePosition.h"
#include "WindowType.h"
#include "BakePassType.h"
#include "ShadowMapIndex.h"
#include "CivilianCarPartIndex.h"
#include "CivilianCarMeshData.h"
#include "CityTileType.h"

class CivilianCarActive : public IRenderable, public IUpdateable
{
private:
    std::vector<glm::ivec2> path;   /// Coordinates of the path.
    std::vector<size_t> dynamicPartIndices;     /// Indices of parts that WON'T be baked (Wheels, Lights)
    std::vector<CivilianCarPart> parts;         /// Vector of parts.
    std::shared_ptr<CivilianCarMeshData> data;  /// Baked data.
    std::array<long long int, static_cast<size_t>(CivilianCarPartIndex::COUNT)> carPartIndex = { -1 }; /// Array of car part indexes.
    size_t currentPathIndex = 0;    /// Current path index.

    Transform transform;    /// Transform.

    float pathUpdateTimer = 0.0f;   /// Update timer.
    float speed = 0.0f;             /// Speed of the car.
    float currentAngle = 0.0f;      /// Yaw rotation (degrees)
    float wheelRotation = 0.0f;     /// Wheel rotation (degrees)
    float steerAngle = 0.0f;        /// Wheel steering angle (degrees)
    float visualRoll = 0.0f;        /// Visual roll (leaning) of the car (degrees)
    
    bool isBraking = false; /// Flag if the car is braking.

    std::vector<std::vector<CityTileType>> cityGrid;    /// City grid.
    glm::vec3 playerPos;                                /// Player position.             

    static constexpr float MAX_SPEED = 50.0f;       /// Max speed of the car.
    static constexpr float ACCELERATION = 13.0f;    /// Acceleration.
    static constexpr float DECELERATION = 10.0f;    /// Deceleration (slowing down)
    static constexpr float TURN_SPEED = 120.0f;     /// Turning speed; degrees per second
    static constexpr float TURN_ANGLE = 40.0f;      /// Turning angle (degrees)
    static constexpr float MAX_ROLL = 5.0f;         /// Max tilt in degrees
    static constexpr float ROLL_STIFFNESS = 4.0f;   /// How fast the car "springs" back

    static constexpr float TURN_ANGLE_INTERPOLATOR = 5.0f;     /// Interpolator value for the turn angle.
    static constexpr float WHEEL_ROTATION_FACTOR = 8.0f;     /// Wheel rotation multiplication factor.

    static constexpr float MOVING_THRESHOLD = 0.1f;     /// Threshold if the car is moving or not.
    static constexpr float STOPPED_THRESHOLD = 0.01f;    /// Threshold if the car is stopped.

    static constexpr float WHEEL_BASE = 2.5f;   /// Wheel base of the car.

    static constexpr float BRAKE_LIGHT_IDLE_INTENSITY = 2.0f;     /// Brake light intensity when not in use.
    static constexpr float BRAKE_LIGHT_USE_INTENSITY = 8.0f;     /// Brake light intensity when in use.

    /**
    * @brief Returns a part with a certain index.
    *
    * @param index Index of the part.
    *
    * @return A pointer to the part, or NULLPTR if it doesn't exist.
    */
    CivilianCarPart* GetPart(CivilianCarPartIndex index);

    /**
    * @brief Returns a part witha  certain index.
    *
    * @param index Index of the part.
    *
    * @return A constant pointer to the part, or NULLPTR if it doesn't exist.
    */
    const CivilianCarPart* GetPart(CivilianCarPartIndex index) const;

    /**
     * @brief Creates a part.
     *
     * @param mesh Unique pointer to the mesh.
     * @param type The mesh type.
     * @param index Index of the mesh.
     * @param offset Offset of the part.
     *
     * @return Reference to the newly-created part, for any other modifications.
     */
    CivilianCarPart& CreatePart(
        std::unique_ptr<Mesh> mesh,
        CivilianCarMeshType type,
        CivilianCarPartIndex index,
        glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Transforms an offset to world coordinates.
    *
    * @param localOffset Vector containing the local part offset.
    */
    glm::vec3 TransformOffsetToWorld(const glm::vec3& localOffset) const;

    /**
    * @brief Creates a base light part.
    *
    * @param shader Weak pointer to the shader.
    * @param lType Type of light to add.
    * @param side What side the light is located on.
    * @param color Color of the mesh / light.
    * @param pos Position of the mesh / light.
    * @param scale Scale of the mesh / light.
    * @param rot Rotation of the mesh / light.
    */
    CivilianCarPart CreateBaseLightPart(
        const std::weak_ptr<Shader>& shader,
        CivilianCarLightType lType,
        FacePosition side,
        const glm::vec3& color,
        const glm::vec3& pos,
        const glm::vec3& scale,
        const glm::vec3& rot
    );


    /**
    * @brief Initialises the police car's parts.
    *
    * The caller, or child classes, should call this, in order to use virtual calls for any override AddXPart(...)
    *
    * @param shader Weak pointer to the shader.
    */
    void InitParts(const std::weak_ptr<Shader>& shader);

    /**
    * @brief Creates the main hollow body of the car.
    *
    * @param shader Weak pointer to the shader.
    * @param color Color of the body.
    * @param pos Position of the body.
    * @param scale Scale of the body.
    * @param rot Rotation of the body.
    */
    void CreateBody(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the cabin of the car.
    *
    * @param shader Weak pointer to the shader.
    * @param alpha Alpha tranparency of the window.
    * @param bodyColor Color of the body parts.
    * @param windowColor Color of the window parts.
    * @param pos Position of the cabin.
    * @param scale Scale of the cabin.
    * @param rot Rotation of the cabin.
    */
    void CreateCabinMain(
        const std::weak_ptr<Shader>& shader,
        uint8_t alpha = 255,
        const glm::vec3& bodyColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& windowColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a seat.
    *
    * @param shader Shader used.
    * @param color Color of the seat
    * @param pos Position of the seat.
    * @param scale Scale of the seat.
    * @param rot Rotation of the seat.
    */
    void CreateSeat(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the wheels
    *
    * @param shader Shader used.
    * @param color Color of the wheels
    * @param pos Position of the wheels.
    * @param scale Scale of the wheels.
    * @param rot Rotation of the wheels.
    */
    void CreateWheels(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the handles
    *
    * @param shader Shader used.
    * @param color Color of the handles
    * @param pos Position of the handles.
    * @param scale Scale of the handles.
    * @param rot Rotation of the handles.
    */
    void CreateHandles(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the bumpers
    *
    * @param shader Shader used.
    * @param color Color of the bumpers
    * @param pos Position of the bumpers.
    * @param scale Scale of the bumpers.
    * @param rot Rotation of the bumpers.
    */
    void CreateBumpers(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the license plates
    *
    * @param shader Shader used.
    * @param color Color of the plates
    * @param pos Position of the plates.
    * @param scale Scale of the plates.
    * @param rot Rotation of the plates.
    */
    void CreatePlates(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the side mirrors
    *
    * @param shader Shader used.
    * @param bodyColor Color of the mirrors' body.
    * @param windowColor Color of the mirrors' mirror.
    * @param pos Position of the mirrors.
    * @param scale Scale of the mirrors.
    * @param rot Rotation of the mirrors.
    */
    void CreateMirrors(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& bodyColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& mirrorColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the grille.
    *
    * @param shader Shader used.
    * @param grileBgColor Color of the grille's background.
    * @param grileColor Color of the grilles.
    * @param pos Position of the grille.
    * @param scale Scale of the grille.
    * @param rot Rotation of the grille.
    */
    void CreateGrille(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& grilleBgColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& grilleColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the cabin side.
    *
    * @param shader Shader used.
    * @param bodyColor Color of the cabin's body.
    * @param windowColor Color of the cabin's windows.
    * @param pos Position of the cabin's side.
    * @param scale Scale of the cabin's side.
    * @param rot Rotation of the cabin's side.
    */
    void CreateCabinSide(
        const std::weak_ptr<Shader>& shader,
        FacePosition side,
        uint8_t alpha,
        const glm::vec3& bodyColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& windowColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the front lights.
    *
    * @param shader Shader used.
    * @param shortBeamColor Color of the short beam.
    * @param highBeamColor Color of the high beam.
    * @param blinkerColor Color of the blinkers.
    * @param pos Position of the front lights.
    * @param scale Scale of the front lights.
    * @param rot Rotation of the front lights.
    */
    void CreateFrontLights(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& shortBeamColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& highBeamColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& blinkerColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Creates the back lights.
    *
    * @param shader Shader used.
    * @param brakeLightColor Color of the brake lights.
    * @param reverLightColor Color of the reverse lights.
    * @param blinkerColor Color of the blinkers.
    * @param pos Position of the back lights.
    * @param scale Scale of the back lights.
    * @param rot Rotation of the back lights.
    */
    void CreateBackLights(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& brakeLightColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& reverLightColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& blinkerColor = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a steering wheel
    *
    * @param shader Shader used.
    * @param color Color of the steering wheel
    * @param pos Position of the steering wheel.
    * @param scale Scale of the steering wheel.
    * @param rot Rotation of the steering wheel.
    */
    void CreateSteeringWheel(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a body part.
    *
    * @param shader Weak pointer to the shader.
    * @param type Type of mesh to add.
    * @param index Index of the mesh to add.
    * @param color Color of the mesh.
    * @param pos Position of the mesh.
    * @param scale Scale of the mesh.
    * @param rot Rotation of the mesh.
    */
    void AddBodyPart(
        const std::weak_ptr<Shader>& shader,
        CivilianCarMeshType type,
        CivilianCarPartIndex index,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a spotlight part.
    *
    * @param shader Weak pointer to the shader.
    * @param lType Type of light to add.
    * @param side What side the light is located on.
    * @param index Index of the part, if any.
    * @param shadowMapIndex Index of this part's shadowMap, as defined in the fragment shader, if any.
    * @param color Color of the mesh.
    * @param pos Position of the mesh.
    * @param scale Scale of the mesh.
    * @param rot Rotation of the mesh.
    */
    void AddSpotLightPart(
        const std::weak_ptr<Shader>& shader,
        CivilianCarLightType lType,
        FacePosition side,
        CivilianCarPartIndex index = CivilianCarPartIndex::None,
        ShadowMapIndex shadowMapindex = ShadowMapIndex::None,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a wheel part.
    *
    * @param shader Weak pointer to the shader.
    * @param index Index of the part, if any.
    * @param color Color of the mesh.
    * @param pos Position of the mesh.
    * @param scale Scale of the mesh.
    * @param rot Rotation of the mesh.
    */
    void AddWheel(
        const std::weak_ptr<Shader>& shader,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a window to the car.
    *
    * @param shader Weak pointer to the shader.
    * @param type Type of window.
    * @param index Index of the part, if any.
    * @param alpha Alpha (transparency) of the window. 255 for opaque, 0 for fully transparent.
    * @param color Color of the mesh.
    * @param pos Position of the mesh.
    * @param scale Scale of the mesh.
    * @param rot Rotation of the mesh.
    */
    void AddWindowPart(
        const std::weak_ptr<Shader>& shader,
        WindowType type,
        uint8_t alpha = 255,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Calculates the wheel matrix for a certain wheel part.
    *
    * This is needed because we need to rotate before scaling.
    *
    * @param part Wheel part.
    *
    * @return A 4x4 matrix representing the transformation matrix of the wheel.
    */
    glm::mat4 CalculateWheelMatrix(const CivilianCarPart& part) const;

    /**
    * @brief Updates the physics.
    *
    * @param deltaTime The delta time of the main GLFW loop.
    */
    void UpdatePhysics(double deltaTime);

    /**
    * @brief Converts grid to world position.
    * 
    * @param gridPos Position on the grid.
    * 
    * @return A vector representing the world coordinates.
    */
    glm::vec3 GridToWorld(const glm::ivec2& gridPos) const;

    /**
    * @brief Converts world to grid position
    *
    * @param worldPos The world position.
    *
    * @return A vector representing the grid coordinates.
    */
    glm::ivec2 WorldToGrid(const glm::vec3& worldPos) const;

    /**
    * @brief Flees from a target
    * 
    * @param playerGrid The grid position of the player.
    * @param grid The city grid.
    * 
    * @return A vector of the next destination to move towards.
    */
    glm::ivec2 FindFleeTarget(glm::ivec2 playerGrid, const std::vector<std::vector<CityTileType>>& grid);

    /**
    * @brief Calculates the path between two points (BFS)
    * 
    * @param start Start point.
    * @param target Destination point.
    * @param grid City grid.
    * 
    * @return A vector of points to go through to reach the destination.
    */
    std::vector<glm::ivec2> CalculatePath(glm::ivec2 start, glm::ivec2 target, const std::vector<std::vector<CityTileType>>& grid);

    /**
    * @brief Updates the wheel transform.
    *
    * @param part The wheel part.
    */
    void UpdateWheelTransform(CivilianCarPart& part) const;

    /**
    * @brief Updates the steering wheel transform.
    *
    * @param part The steering wheel part.
    */
    void UpdateSteeringTransform(CivilianCarPart& part) const;
    
    /**
    * @brief Handles a spot light.
    *
    * @param part Reference to the civilian car part containing the spot light.
    */
    void HandleSpotLight(CivilianCarPart& part) const;

    /**
    * @brief Bakes the static car parts into a single mesh.
    *
    * @param shader Weak pointer to the shader used.
    */
    void Bake(const std::weak_ptr<Shader>& shader);

    /**
    * @brief Performs a bake pass.
    *
    * @param shader Weak pointer to the shader.
    * @param passType The bake pass type.
    *
    * @return A unique pointer to the baked mesh created.
    */
    std::unique_ptr<Mesh> PerformBakePass(
        const std::weak_ptr<Shader>& shader,
        BakePassType passType
    );

public:

    /**
    * @brief Constructor.
    * 
    * @param shader Weak pointer to the shader.
    * @param cityGrid The grid of the city.
    * @param t Optional transform
    */
    CivilianCarActive(
        const std::weak_ptr<Shader>& shader,
        std::vector<std::vector<CityTileType>> cityGrid = {},
        const Transform& t = {}
    );

    /**
    * @brief Default destructor.
    */
    ~CivilianCarActive() override = default;

    /**
    * @brief Updates the car.
    *
    * @param deltaTime Time elapsed since last frame (seconds)
    */
    void Update(double deltaTime) override;

    /**
    * @brief Draws the car.
    * 
    * @param customShader Custom shader. If NULLPTR, it will use meshes' shader.
    */
    void Draw(std::shared_ptr<Shader> customShader = nullptr) const override;

    /**
    * @brief Returns all lights.
    *
    * @return Vector containing pointers to all lights
    */
    std::vector<CivilianCarLight*> GetAllLights();

    /**
    * @brief Returns the foward vector of the car.
    *
    * This assumes the car starts facing the +Z axis.
    *
    * @return The forward vector of the car.
    */
    glm::vec3 GetForward() const;

    /**
    * @brief Returns the positon of the car.
    * 
    * @return The position of the car.
    */
    glm::vec3 GetPosition() const;

    /**
    * @brief Gets the world position of a certain light.
    *
    * @param partIndex The part index of the light.
    *
    * @return The world position of the light.
    */
    glm::vec3 GetWorldPosOfLight(CivilianCarPartIndex partIndex) const;

    /**
    * @brief Gets the spotlight angle in radians for a certain spot light.
    *
    * @param partIndex The part index of the spot light.
    *
    * @return The spotlight outer angle in radians.
    */
    float GetSpotlightOuterAngle(CivilianCarPartIndex partIndex) const;

    /**
    * @brief Sets the position of the player for the civilian car to use.
    * 
    * @param playerPos The position of the player.
    */
    void SetPlayerPosition(const glm::vec3& playerPos);
};

#endif // CIVILIANCARACTIVE_H_