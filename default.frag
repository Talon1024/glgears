#version 330 core

uniform bool lit;

// Calculated in the vertex shader
in vec4 lightColour;
in vec4 diffuse;
//in vec3 vNrm;

void main() {
	gl_FragColor = lit ? lightColour : diffuse;
	//gl_FragColor = vec4(vNrm, 1.);
}