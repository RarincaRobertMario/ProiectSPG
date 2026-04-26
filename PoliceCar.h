#pragma once

#ifndef POLICECAR_H_
#define POLICECAR_H_

#include <array>

#include "Mesh.h"
#include "IUpdateable.h"
#include "PoliceCarMeshType.h"
#include "PoliceCarPart.h"
#include "Direction.h"
#include "PoliceCarPartIndex.h"
#include "ShadowMapIndex.h"
#include "BlinkerSide.h"

struct vec3;

/**
* @brief A police car. Can move and has a siren.
*/
class PoliceCar : public IUpdateable, public IRenderable
{
private:
    mutable std::vector<PoliceCarPart> parts;   /// Vector of parts for the car.
    std::array<long long int, static_cast<size_t>(PoliceCarPartIndex::COUNT)> carPartIndex = { -1 };    /// Array of [index in parts, PoliceCarPartIndex]
    double sirenTimer = 0.0;    /// Timer for the flashing lights.
    double blinkerTimer = 0.0;  /// Timer for the blinker lights.

    std::shared_ptr<Mesh> bakedBodyMesh;
    std::vector<size_t> dynamicPartIndices; // Indices of parts that WON'T be baked (Wheels, Lights)

    Transform transform;        /// Transformation structure for the car. Added ontop of the mesh ones.

    float speed = 0.0f;         /// Speed of the car.
    float maxSpeed = 20.0f;     /// Max speed of the car.
    float acceleration = 15.0f; /// Acceleration.
    float deceleration = 10.0f; /// Deceleration (slowing down)
    float turnSpeed = 100.0f;   /// Turning speed; degrees per second
    float currentAngle = 0.0f;  /// Yaw rotation (degrees)
    float wheelRotation = 0.0f; /// Wheel rotation (degrees)
    float steerAngle = 0.0f;    /// Wheel steering angle (degrees)
    bool isBraking = false;     /// Flag if the car is braking.
    bool isReversing = false;   /// Flag if the car is reversing.

    BlinkerSide blinkerSide;    /// What blinkers are currently on.
    bool isLightBarOn = false;  /// Flag if the lightbars are on.
    bool areBrakeLightsOn = true;   /// Flag if the brakelights are on.

    /**
    * @brief Initialises the police car.
    * 
    * @param shader Weak pointer to the shader.
    */
    void Init(const std::weak_ptr<Shader>& shader);

    /**
    * @brief Adds a body part.
    * 
    * @param shader Weak pointer to the shader.
    * @param type Type of mesh to add.
    * @param index Index of the part, if any.
    * @param color Color of the mesh.
    * @param pos Position of the mesh.
    * @param scale Scale of the mesh.
    * @param rot Rotation of the mesh.
    */
    void AddBodyPart(
        const std::weak_ptr<Shader>& shader,
        PoliceCarMeshType type, 
        PoliceCarPartIndex index = PoliceCarPartIndex::None, 
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Adds a body part.
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
    void AddLightPart(
        const std::weak_ptr<Shader>& shader,
        PoliceCarLightType lType, 
        FacePosition side,
        PoliceCarPartIndex index = PoliceCarPartIndex::None,
        ShadowMapIndex shadowMapindex = ShadowMapIndex::None,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
     * @brief Adds a point light.
     *
     * @param shader Weak pointer to the shader.
     * @param lType Type of light to add.
     * @param side What side the light is located on.
     * @param index Index of the part, if any.
     * @param shadowCubeIndex Index of this part's shadow cube, as defined in the fragment shader, if any.
     * @param color Color of the mesh.
     * @param pos Position of the mesh.
     * @param scale Scale of the mesh.
     * @param rot Rotation of the mesh.
     */
    void AddPointLightPart(
        const std::weak_ptr<Shader>& shader,
        PoliceCarLightType lType,
        FacePosition side,
        PoliceCarPartIndex index = PoliceCarPartIndex::None,
        ShadowCubeIndex shadowCubeIndex = ShadowCubeIndex::None,
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
        PoliceCarPartIndex index, 
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    );

    /**
    * @brief Something about a Builder pattern (I am lying).
    * 
    * @param shader Weak pointer to the shader used.
    */
    void Bake(const std::weak_ptr<Shader>& shader);

    /**
    * @brief Handles a point light.
    * 
    * @param part Reference to the police car part containing the point light.
    */
    void HandlePointLight(PoliceCarPart& part) const;

    /**
    * @brief Handles a spot light.
    * 
    * @param part Reference to the police car part containing the spot light.
    */
    void HandleSpotLight(PoliceCarPart& part) const;

public:
    
    /**
    * @brief Constructor.
     * 
     * Do note that that you cannot set the overral color of the car.
     * 
     * @param shader Weak pointer to the shader used.
     * @param transform Transformation structure for the car. Added ontop of the mesh ones.
     */
    PoliceCar(
        const std::weak_ptr<Shader>& shader,
        const Transform& transform = {}
    );

    /**
    * @brief Updates the police car.
    * 
    * @param deltaTime Time elapsed since last frame (seconds)
    */
    void Update(double deltaTime) override;

    /**
    * @brief Draws the police car.
    * 
    * @param customShader Custom shader to use. Nullptr to use meshes' own shaders.
    */
    void Draw(std::shared_ptr<Shader> customShader = nullptr) const override;

    /**
    * @brief Processes keyboard input.
    * 
    * @param direction Direction of movement.
    * @param deltaTime The delta time of the main GL loop.
    */
    void ProcessKeyboard(Direction direction, float deltaTime);

    /**
    * @brief Toggles the high beams.
    * 
    * Initially they are turned off.
    */
    void ToggleHighBeams();

    /**
    * @brief Toggles the short beams.
    * 
    * Initially they are turned on.
    */
    void ToggleShortBeams();

    /**
    * @brief Sets the blinker state for the car.
    * 
    * @param side Side to set the blinker state to. If it's the same as the currently 'on' side, it switches off.
    */
    void SetBlinkerState(BlinkerSide side);

    /**
    * @brief Toggles the light bar.
    * 
    * Initially off.
    */
    void ToggleLightBar();

    /**
    * @brief Toggles rear lights.
    * 
    * Initially on.
    */
    void ToggleRearLights();

    /**
    * @brief Returns all lights.
    * 
    * @return Vector containing pointers to all lights
    */
    std::vector<PoliceCarLight *> GetAllLights();

    /**
    * @brief Returns the foward vector of the car.
    * 
    * @return The forward vector of the car.
    */
    glm::vec3 GetForward() const;

    /**
    * @brief Gets the world position of a certain light.
    * 
    * @param partIndex The part index of the light.
    * 
    * @return The world position of the light.
    */
    glm::vec3 GetWorldPosOfLight(PoliceCarPartIndex partIndex) const;

    /**
    * @brief Gets the spotlight angle in radians for a certain spot light.
    *
    * @param partIndex The part index of the spot light.
    *
    * @return The spotlight outer angle in radians.
    */
    float GetSpotlightOuterAngle(PoliceCarPartIndex partIndex) const;
};

#endif // POLICECAR_H_