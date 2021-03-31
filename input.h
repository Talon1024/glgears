#pragma once
#include <GLFW/glfw3.h>

struct KeyInputState {
	// Keyboard controls
	bool forward;
	bool backward;
	bool turnleft;
	bool left;
	bool turnright;
	bool right;
	bool shift;
	// Program state (toggles)
	bool wireframe;
	bool lit;
	bool animate;
};

struct MouseInputState {
	bool pointerLock;
	double moveX;
	double moveY;
};

namespace Input {
	void onKeyAction(GLFWwindow* window, int k, int s, int action, int mods);
	void onMouseMove(GLFWwindow* window, double xpos, double ypos);
	void onMouseButton(GLFWwindow* window, int button, int action, int mods);
	const KeyInputState* GetKeyState();
	const MouseInputState* GetMouseState();
}