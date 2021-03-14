#include "input.h"
#include <GLFW/glfw3.h>

static InputStatus curInputStatus {false, false, false, false, false, false, false, true, true};

/* change view angle, exit upon ESC */
void Input::key( GLFWwindow* window, int k, int s, int action, int mods )
{
  // Toggles when key is pressed or released
  switch (k) {
  case GLFW_KEY_Z:
    curInputStatus.z = action != GLFW_RELEASE;
    break;
  case GLFW_KEY_UP:
    curInputStatus.up = action != GLFW_RELEASE;
    break;
  case GLFW_KEY_DOWN:
    curInputStatus.down = action != GLFW_RELEASE;
    break;
  case GLFW_KEY_LEFT:
    curInputStatus.left = action != GLFW_RELEASE;
    break;
  case GLFW_KEY_RIGHT:
    curInputStatus.right = action != GLFW_RELEASE;
    break;
  case GLFW_KEY_LEFT_SHIFT:
  case GLFW_KEY_RIGHT_SHIFT:
    curInputStatus.shift = action != GLFW_RELEASE;
    break;
  }

  // Toggles when key is pressed
  if (action != GLFW_PRESS) return;

  switch (k) {
  case GLFW_KEY_ESCAPE:
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    break;
  case GLFW_KEY_V:
    curInputStatus.wireframe = !curInputStatus.wireframe;
    break;
  case GLFW_KEY_L:
    curInputStatus.lit = !curInputStatus.lit;
    break;
  case GLFW_KEY_T:
    curInputStatus.animate = !curInputStatus.animate;
    break;
  }
}

const InputStatus* Input::GetInputStatus()
{
	return &curInputStatus;
}