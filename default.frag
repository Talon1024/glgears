#version 330 core

// Calculated in the vertex shader
in vec4 lightColour;
in vec4 diffuse;

void main() {
	gl_FragColor = lightColour;
}