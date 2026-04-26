#pragma once

#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#include "PoliceCarActive.h"
#include "Direction.h"
#include "CameraMode.h"

/**
* @brief Simple camera with position, look-at, and projection, can move.
*/
class Camera : public IUpdateable
{
private:
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };  /// Position of the camera.
    glm::vec3 front = { 0.0f, 0.0f,0.0f };      /// Front of the camera (point where the camera looks at).
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };        /// Where 'up' is.
    glm::vec3 right = { 1.0f, 0.0f, 0.0f };     /// Where 'right' is
    glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };   /// Where 'global up' is (the camera 'up' might change)

    CameraMode mode = CameraMode::Follow;     /// Camera mode.
    std::weak_ptr<PoliceCarActive> target;  /// Target pointer.

    float followDistance = 12.0f;   /// Follow distance for 3rd person.
    float followHeight = 4.0f;      /// Follow height for 3rd person.
    float lerpSpeed = 5.0f;         /// LERP speed

    float yaw = -90.0f;     /// Yaw of the camera.
    float pitch = 0.0f;     /// Pitch of the camera
    float movementSpeed = 15.0f;     /// Move speed of the camera.
    float mouseSensitivity = 0.1f;  /// Camera sensitivity

    float fov = 60.0f;              /// Field-of-view.
    float aspect = 10.0f;           /// Aspect ratio.
    float nearPlane = 0.1f;         /// Near plane.
    float farPlane = 100.0f;        /// Far plane.

    double lastX = 400.0;           /// Last X mouse position.
    double lastY = 300.0;           /// Last Y mouse position.

    /**
    * @brief Updates the camera vectors.
    */
    void UpdateCameraVectors();

    /**
    * @brief Updates the follow mode.
    * 
    * @param t Target to follow.
    * @param deltaTime The delta time of the main GLFW loop.
    */
    void UpdateFollowMode(std::shared_ptr<PoliceCarActive> t, double deltaTime);

    /**
    * @brief Updates the first person mode.
    *
    * @param t Target to follow.
    * @param deltaTime The delta time of the main GLFW loop.
    */
    void UpdateFirstPersonMode(std::shared_ptr<PoliceCarActive> t, double deltaTime);

public:

    /**
    * @brief Default constructor.
    */
    Camera() = default;

    /**
    * @brief Constructor.
    * 
    * @param pos Position of the camera.
    * @param target Target of the camera.
    * @param worldUp Where the 'up' direction is.
    * @param fovDeg Field-of-view in degrees.
    * @param aspectRatio The aspect ratio of the camera.
    * @param nearP The near plane of the camera.
    * @param farP The far plane of the camera.
    */
    Camera(
        const glm::vec3& pos, 
        const glm::vec3& target, 
        const glm::vec3& worldUp,
        float fovDeg, 
        float aspectRatio, 
        float nearP = 0.1f, 
        float farP = 100.0f
    );

    /**
    * @brief Updates the camera
    * 
    * @param deltaTime The delta time of the main GLFW loop.
    */
    void Update(double deltaTime) override;

    /**
    * @brief Processes keyboard presses.
    * 
    * @param direction Direction to move the camera in.
    * @param deltaTime The delta time of the main GL loop.
    */
    void ProcessKeyboard(Direction direction, float deltaTime);

    /**
    * @brief Processes mouse movements.
    * 
    * @param xPos X position of the mouse.
    * @param yPos Y position of the mouse.
    * @param constrainPitch If the pitch should be kept between -90 and 90 degrees.
    */
    void ProcessMouseMovement(float xPos, float yPos, bool constrainPitch = true);

    /**
    * @brief Returns the view matrix of the camera.
    * 
    * @return The view matrix (4x4) of the camera.
    */
    glm::mat4 GetViewMatrix() const;

    /**
    * @brief Returns the projection matrix of the camera.
    * 
    * @return The projection matrix (4x4) of the camera.
    */
    glm::mat4 GetProjectionMatrix() const;

    /**
    * @brief Returns the camera position.
    * 
    * @return Constant reference to the camera position.
    */
    const glm::vec3& GetPosition() const;

    /**
    * @brief Sets the camera position.
    *
    * @param newPosition New position for the camera
    */
    void SetPosition(const glm::vec3& newPosition);

    /**
    * @brief Returns the camera target ('front').
    *
    * @return Constant reference to the camera target.
    */
    const glm::vec3& GetFront() const;

    /**
    * @brief Sets the camera target ('front').
    *
    * @param newFront New target ('front') of the camera.
    */
    void SetFront(const glm::vec3& newFront);

    /**
    * @brief Returns the camera's up direction.
    *
    * @return Constant reference to the camera's up direction.
    */
    const glm::vec3& GetUp() const;

    /**
    * @brief Sets the camera mode.
    * 
    * @param newMode New camera mode.
    */
    void SetMode(CameraMode newMode);

    /**
    * @brief Sets the target of the camera.
    * 
    * @param newTarget Weak pointer to the new target.
    */
    void SetTraget(const std::weak_ptr<PoliceCarActive> newTarget);
};

#endif // CAMERA_H_