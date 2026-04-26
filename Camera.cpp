#include <iostream>
#include <glm/glm.hpp>

#include "Camera.h"

Camera::Camera(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& worldUp, float fovDeg, float aspectRatio, float nearP, float farP) :
    position(pos),
    worldUp(worldUp),
    up(worldUp),
    fov(fovDeg),
    aspect(aspectRatio),
    nearPlane(nearP),
    farPlane(farP)
{
    front = glm::normalize(target - pos);
    UpdateCameraVectors();
}

void Camera::Update(double deltaTime)
{
    if (mode == CameraMode::FreeRoam)
    {
        return;
    }

    auto lockedTarget = target.lock();
    if (!lockedTarget)
    {
        std::cerr << "{Camera::Update} ERROR: Target expired or not set." << std::endl;
        return;
    }

    if (mode == CameraMode::Follow) 
    {
        UpdateFollowMode(lockedTarget, deltaTime);
    }
    else if (mode == CameraMode::FirstPerson) 
    {
        UpdateFirstPersonMode(lockedTarget, deltaTime);
    }
}
void Camera::ProcessKeyboard(Direction direction, float deltaTime)
{
    if (mode != CameraMode::FreeRoam)
    {
        return;
    }

    nearPlane = 0.1f;

    float velocity = movementSpeed * deltaTime;
    if (direction == Forward)  position += front * velocity;
    if (direction == Backward) position -= front * velocity;
    if (direction == Left)     position -= right * velocity;
    if (direction == Right)    position += right * velocity;
}

void Camera::ProcessMouseMovement(float xpos, float ypos, bool constrainPitch)
{
    if (mode != CameraMode::FreeRoam)
    {
        return;
    }

    nearPlane = 0.1f;

    float xoffset = static_cast<float>(xpos - lastX);
    float yoffset = static_cast<float>(lastY - ypos);
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch)
    {
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }

    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
}

const glm::vec3& Camera::GetPosition() const { return position; }

void Camera::SetPosition(const glm::vec3& newPosition) { position = newPosition; }

const glm::vec3& Camera::GetFront() const { return front; }

void Camera::SetFront(const glm::vec3& newFront) { front = newFront; }

const glm::vec3& Camera::GetUp() const { return up; }

void Camera::SetMode(CameraMode newMode) { mode = newMode; }

void Camera::SetTraget(const std::weak_ptr<PoliceCarActive> newTarget) { target = newTarget; }

void Camera::UpdateCameraVectors()
{
    glm::vec3 newFront{};
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    // Also re-calculate the Right and Up vector
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::UpdateFollowMode(std::shared_ptr<PoliceCarActive> t, double deltaTime)
{
    glm::vec3 targetPos = t->GetPosition();
    glm::vec3 targetForward = t->GetForward();

    glm::vec3 desiredPos = targetPos - (targetForward * followDistance) + (glm::vec3(0, 1, 0) * followHeight);
    position = glm::mix(position, desiredPos, lerpSpeed * deltaTime);
    //front = glm::normalize(targetPos - position);
    front = glm::normalize((targetPos + targetForward * 5.0f) - position);

    // Re-calculate vectors
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::UpdateFirstPersonMode(std::shared_ptr<PoliceCarActive> t, double deltaTime)
{
glm::vec3 targetPos = t->GetPosition();
    glm::vec3 rot = t->GetRotation(); // (pitch, yaw, roll)

    glm::mat4 carRotation = glm::mat4(1.0f);
    carRotation = glm::rotate(carRotation, rot.y, glm::vec3(0, 1, 0)); // Yaw
    carRotation = glm::rotate(carRotation, rot.z, glm::vec3(0, 0, 1)); // Roll (Lean)
    carRotation = glm::rotate(carRotation, rot.x, glm::vec3(1, 0, 0)); // Pitch (Braking)

    glm::vec3 localForward = glm::normalize(glm::vec3(carRotation * glm::vec4(0, 0, 1, 0)));
    glm::vec3 localUp      = glm::normalize(glm::vec3(carRotation * glm::vec4(0, 1, 0, 0)));
    glm::vec3 localRight   = glm::normalize(glm::vec3(carRotation * glm::vec4(1, 0, 0, 0)));

    position = targetPos + (localForward * -0.1f) + (localUp * 1.15f) - (localRight * -0.5f);

    front = localForward;
    up    = localUp;
    right = localRight;

    nearPlane = 0.5f;
}