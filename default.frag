#version 330 core

uniform bool lit;

// Calculated in the vertex shader
in vec4 lightColour;
in vec4 diffuse;
// in vec3 vNrm;
in float distanceFromCamera;

void main() {
	// gl_FragColor = lit ? lightColour : diffuse;
	vec4 grayShade = vec4(vec3(distanceFromCamera / 50.), 1.);
	gl_FragColor = lit ? lightColour : grayShade * diffuse;
	// gl_FragColor = vec4(vNrm, 1.);
}