#include <GLFW/glfw3.h>

#include "KeyState.h"

bool KeyState::JustPressed(GLFWwindow* window, int key)
{
    isPressed = (glfwGetKey(window, key) == GLFW_PRESS);
    bool result = (isPressed && !wasPressed);
    wasPressed = isPressed;
    return result;
}