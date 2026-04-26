#pragma once

#ifndef KEYSTATE_H_
#define KEYSTATE_H_

struct GLFWwindow;

/**
* @brief Structure for a key state.
*/
struct KeyState 
{
    bool isPressed = false;     /// Flag if the key is pressed.
    bool wasPressed = false;    /// Flag if the previous 'time' the key was pressed.

    /**
    * @brief Checks if the key was just pressed.
    * 
    * @param window Pointer to the OpenGL window.
    * @param key Key to check.
    * 
    * @return 'True' if this is the FIRST frame of the key being pressed down, 'false' for all subsequent frames until key is no longer pressed.
    */
    bool JustPressed(GLFWwindow* window, int key);
};

#endif // KEYSTATE_H_