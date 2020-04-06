#version 330 core

uniform vec3 lightPos;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNrm;
layout(location = 2) in vec3 aCol;

out vec4 lightColour;
// out float lightIntensity;

void main() {
	// NOTE: This is per-vertex lighting. It's faster, but doesn't look as good
	// as per-pixel lighting. However, since the gears have no smooth faces,
	// per-pixel lighting is really not necessary.
	vec3 lightDiff = normalize(aPos - lightPos);
	float lightIntensity = dot(lightDiff, aNrm);
	lightColour = vec4(aCol, 1.) * lightIntensity;
	gl_Position = aPos * modelView * projection;
}