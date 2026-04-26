#pragma once

#ifndef CAMERAMODE_H_
#define CAMERAMODE_H_

/**
* @brief Enumeration of camera modes.
*/
enum class CameraMode 
{
    FreeRoam,       /// Free roam camera.
    Follow,         /// Third person camera (behind and up).
    FirstPerson,    /// First person camera.
    COUNT           /// Number of elements.
};

#endif // CAMERAMODE_H_