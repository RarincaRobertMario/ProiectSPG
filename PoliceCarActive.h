#pragma once

#ifndef POLICECARACTIVE_H_
#define POLICECARACTIVE_H_

#include "PoliceCarModel.h"
#include "IUpdateable.h"
#include "BlinkerSide.h"
#include "Direction.h"

/**
* @brief An 'active' police car, updates, has lights.
*/
class PoliceCarActive : public PoliceCarModel, public IUpdateable
{
private:
    double sirenTimer = 0.0;    /// Timer for the flashing lights.
    double blinkerTimer = 0.0;  /// Timer for the blinker lights.
    std::vector<size_t> dynamicPartIndices; // Indices of parts that WON'T be baked (Wheels, Lights)

    float speed = 0.0f;         /// Speed of the car.
    float currentAngle = 0.0f;  /// Yaw rotation (degrees)
    float wheelRotation = 0.0f; /// Wheel rotation (degrees)
    float steerAngle = 0.0f;    /// Wheel steering angle (degrees)
    float visualRoll = 0.0f;    /// Visual roll (leaning left-right) of the car (degrees)
    float visualPitch = 0.0f;   /// Visual pitch (leaning forward-backwards) of the car (degrees)

    BlinkerSide blinkerSide = BlinkerSide::None;    /// What blinkers are currently on.

    /**
    * @brief Structure for car flags.
    */
    struct CarFlags
    {
        bool isBraking : 1;         /// Flag if the car is braking.
        bool isReversing : 1;       /// Flag if the car is reversing.
        bool isLightBarOn : 1;      /// Flag if the lightbars are on.
        bool brakeLightsOn : 1;     /// Flag if the brakelights are on.
        bool headlightsOn : 1;      /// Flag if the headlights are on.
        bool highBeamsOn : 1;       /// Flag if the high beams are on.
        uint8_t unused : 2;         /// Unused.
    };

    /// Structure for the car flags.
    CarFlags flags = { 
        .isBraking = false,
        .isReversing = false,
        .isLightBarOn = false,
        .brakeLightsOn = true,
        .headlightsOn = true,
        .highBeamsOn = false,
        .unused = 0
    };

    static constexpr float MAX_SPEED    = 50.0f;    /// Max speed of the car.
    static constexpr float ACCELERATION = 15.0f;    /// Acceleration.
    static constexpr float DECELERATION = 10.0f;    /// Deceleration (slowing down)
    static constexpr float TURN_SPEED   = 100.0f;   /// Turning speed; degrees per second
    static constexpr float TURN_ANGLE   = 30.0f;    /// Turning angle (degrees)
    static constexpr float MAX_ROLL     = 5.0f;     /// Max tilt in degrees
    static constexpr float ROLL_STIFFNESS = 4.0f;   /// How fast the car "springs" back

    static constexpr float TURN_ANGLE_INTERPOLATOR  = 5.0f;     /// Interpolator value for the turn angle.
    static constexpr float WHEEL_ROTATION_FACTOR    = 8.0f;     /// Wheel rotation multiplication factor.

    static constexpr float MOVING_THRESHOLD     = 0.1f;     /// Threshold if the car is moving or not.
    static constexpr float STOPPED_THRESHOLD    = 0.01f;    /// Threshold if the car is stopped.

    static constexpr float WHEEL_BASE = 2.5f;   /// Wheel base of the car.

    static constexpr float HIGHBEAM_INTENSITY           = 10.0f;    /// High beam light intensity.
    static constexpr float SHORTBEAM_INTENSITY          = 5.0f;     /// Short beam light intensity.
    static constexpr float REVERSE_LIGHT_INTENSITY      = 4.0f;     /// Reverse light intensity when in use.
    static constexpr float BRAKE_LIGHT_IDLE_INTENSITY   = 2.0f;     /// Brake light intensity when not in use.
    static constexpr float BRAKE_LIGHT_USE_INTENSITY    = 8.0f;     /// Brake light intensity when in use.
    static constexpr float LIGHTBAR_INTENSITY           = 20.0f;    /// Light bar intensity when in use.
    static constexpr float BLINKER_INTENSITY            = 1.0f;     /// Blinker intensity when in use.

    static constexpr float ACTIVE_LIGHT_MESH_INTENSITY      = 1.0f;     /// Light mesh intensity when active.
    static constexpr float INACTIVE_LIGHT_MESH_INTENSITY    = 0.05f;    /// Light mesh intensity when not active (slightly glowing to simulate reflectiveness)

    static constexpr float LIGHTBAR_PULSE_SPEED = 2.0f; /// Light bar pulse speed.
    static constexpr float BLINKER_PULSE_SPEED  = 3.0f; /// Blinker pulse speed.

    static constexpr float HIGHBEAM_INNER_CUTOFF = 45.0f;   /// High beam inner cut-off angle.
    static constexpr float HIGHBEAM_OUTER_CUFOFF = 60.0f;   /// High beam outer cut-off angle.

    /**
    * @brief Calculates the wheel matrix for a certain wheel part.
    * 
    * This is needed because we need to rotate the wheels a certain way.
    * 
    * @param part Wheel part.
    * 
    * @return A 4x4 matrix representing the transformation matrix of the wheel.
    */
    glm::mat4 CalculateWheelMatrix(const PoliceCarPart& part) const;

    /**
    * @brief Calculates the lean matrix.
    * 
    * @return The lean rotation matrix.
    */
    glm::mat4 CalculateLeanMatrix() const;

    /**
    * @brief Updates the physics.
    * 
    * @param deltaTime The delta time of the main GLFW loop.
    */
    void UpdatePhysics(double deltaTime);

    /**
    * @brief Updates the wheel transform.
    * 
    * @param part The wheel part.
    */
    void UpdateWheelTransform(PoliceCarPart& part) const;

    /**
    * @brief Updates the steering wheel transform.
    *
    * @param part The steering wheel part.
    */
    void UpdateSteeringTransform(PoliceCarPart& part) const;

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
    PoliceCarPart CreateBaseLightPart(
        const std::weak_ptr<Shader>& shader,
        PoliceCarLightType lType,
        FacePosition side,
        const glm::vec3& color,
        const glm::vec3& pos,
        const glm::vec3& scale,
        const glm::vec3& rot
    );

    /**
    * @brief Sets the light state of a certain part.
    * 
    * @param index Part index of the light.
    * @param active 'true' if the light should be active, 'false' otherwise.
    * @param intensity The intensity of the light, if active. If not active, intensity is 0.0f.
    */
    void SetLightState(PoliceCarPartIndex index, bool active, float intensity);

    /**
    * @brief Transforms an offset to world coordinates.
    *
    * @param localOffset Vector containing the local part offset.
    */
    glm::vec3 TransformOffsetToWorld(const glm::vec3& localOffset) const;

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

    /**
    * @brief Updates base light properties.
    * 
    * @param base Reference to the base light properties.
    * @param part Reference to the car part.
    */
    void UpdateBaseProperties(LightProperties& base, PoliceCarPart& part) const;

    /**
    * @brief Calculates the light state.
    * 
    * @param part Reference to the car part.
    * @param [out] outIntensity The intensity of the light.
    * 
    * @return 'true' if the light should be on, 'false' otherwise.
    */
    bool CalculateLightState(const PoliceCarPart& part, float& outIntensity) const;

protected:

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
        PoliceCarLightType lType,
        FacePosition side,
        PoliceCarPartIndex index = PoliceCarPartIndex::None,
        ShadowMapIndex shadowMapindex = ShadowMapIndex::None,
        const glm::vec3& color = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& pos = glm::vec3(0.0f, 0.0f, 0.0f),
        const glm::vec3& scale = glm::vec3(1.0f, 1.0f, 1.0f),
        const glm::vec3& rot = glm::vec3(0.0f, 0.0f, 0.0f)
    ) override;

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
    ) override;

    /**
    * @brief Performs a bake pass.
    * 
    * @param shader Pointer to the shader.
    * @param passType Type of bake pass.
    */
    std::unique_ptr<Mesh> PerformBakePass(const std::weak_ptr<Shader>& shader, BakePassType passType) override;

public:

    /**
    * @brief Constructor.
     *
     * Do note that that you cannot set the overral color of the car.
     *
     * @param shader Weak pointer to the shader used.
     * @param transform Transformation structure for the car. Added ontop of the mesh ones.
     */
    PoliceCarActive(
        const std::weak_ptr<Shader>& shader,
        const Transform& transform = {}
    );

    /**
    * @brief Default destructor.
    */
    ~PoliceCarActive() override = default;

    /**
    * @brief Updates the police car.
    *
    * @param deltaTime Time elapsed since last frame (seconds)
    */
    void Update(double deltaTime) override;

    /**
    * @brief Draws the police car.
    *
    * 'OnDraw' should be overloaded instead.
    *
    * @param customShader Custom shader to use. Nullptr to use meshes' own shaders.
    */
    virtual void Draw(std::shared_ptr<Shader> customShader = nullptr) const override;

    /**
    * @brief Processes keyboard input.
    *
    * @param direction Direction of movement.
    * @param deltaTime The delta time of the main GL loop.
    */
    void ProcessKeyboard(Direction direction, double deltaTime);

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
    std::vector<PoliceCarLight*> GetAllLights();

    /**
    * @brief Returns the foward vector of the car.
    *
    * This assumes the car starts facing the +Z axis.
    * 
    * @return The forward vector of the car.
    */
    glm::vec3 GetForward() const;

    /**
    * @brief Returns the position of the car.
    * 
    * @return The position vector of the car.
    */
    glm::vec3 GetPosition() const;

    /**
    * @brief Returns the rotation of the car.
    *
    * @return The rotation vector of the car.
    */
    glm::vec3 GetRotation() const;

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

#endif // POLICECARACTIVE_H_