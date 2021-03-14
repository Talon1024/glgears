#pragma once
#include <GLFW/glfw3.h>

struct InputStatus {
	bool up;
	bool down;
	bool left;
	bool right;
	bool shift;
	bool z;
	bool wireframe;
	bool lit;
	bool animate;
};

namespace Input {
	void key( GLFWwindow* window, int k, int s, int action, int mods );
	const InputStatus* GetInputStatus();
}