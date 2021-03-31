#include "input.h"
#include <GLFW/glfw3.h>

static KeyInputState curKeyState {
	// Keyboard controls
	false, // bool forward;
	false, // bool backward;
	false, // bool turnleft;
	false, // bool left;
	false, // bool turnright;
	false, // bool right;
	false, // bool shift;
	// Program state (toggles)
	false, // bool wireframe;
	true, // bool lit;
	true, // bool animate;
};

static MouseInputState curMouseState {
	false, // bool pointerLock;
	0, // double moveX;
	0, // double moveY;
};

/* change view angle, exit upon ESC */
void Input::onKeyAction( GLFWwindow* window, int k, int s, int action, int mods )
{
  bool on = action != GLFW_RELEASE;
  // Toggles when key is pressed or released
  switch (k) {
  case GLFW_KEY_UP:
  case GLFW_KEY_W:
    curKeyState.forward = on;
    break;
  case GLFW_KEY_DOWN:
  case GLFW_KEY_S:
    curKeyState.backward = on;
    break;
  case GLFW_KEY_A:
    curKeyState.left = on;
    break;
  case GLFW_KEY_LEFT:
    curKeyState.turnleft = on;
    break;
  case GLFW_KEY_D:
    curKeyState.right = on;
    break;
  case GLFW_KEY_RIGHT:
    curKeyState.turnright = on;
    break;
  case GLFW_KEY_LEFT_SHIFT:
  case GLFW_KEY_RIGHT_SHIFT:
    curKeyState.shift = on;
    break;
  }

  // Toggles when key is pressed
  if (action != GLFW_PRESS) return;

  switch (k) {
  case GLFW_KEY_ESCAPE:
    if (curMouseState.pointerLock)
    {
      curMouseState.pointerLock = false;
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else
    {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    break;
  case GLFW_KEY_V:
    curKeyState.wireframe = !curKeyState.wireframe;
    break;
  case GLFW_KEY_L:
    curKeyState.lit = !curKeyState.lit;
    break;
  case GLFW_KEY_T:
    curKeyState.animate = !curKeyState.animate;
    break;
  }
}

static double prevXPos = 0, prevYPos = 0, curXPos = 0, curYPos = 0;

void Input::onMouseMove(GLFWwindow *window, double xpos, double ypos)
{
  prevXPos = curXPos;
  prevYPos = curYPos;
  curXPos = xpos;
  curYPos = ypos;
}

void Input::onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
  if (!curMouseState.pointerLock && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
  {
    curMouseState.pointerLock = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }
}

const KeyInputState* Input::GetKeyState()
{
	return &curKeyState;
}

const MouseInputState* Input::GetMouseState()
{
  MouseInputState* state = &curMouseState;
  if (!state->pointerLock)
  {
    state->moveX = state->moveY = 0;
    return state;
  }
  state->moveX = curXPos - prevXPos;
  state->moveY = curYPos - prevYPos;
  curXPos = prevXPos;
  curYPos = prevYPos;
  return state;
}