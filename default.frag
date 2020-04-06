#version 330 core

// Calculated in the vertex shader
in vec4 lightColour;

void main() {
	gl_FragColor = lightColour;
}