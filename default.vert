#version 330 core

uniform vec3 lightPos;
uniform vec3 colour;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform float zoom;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNrm;
layout(location = 2) in vec3 aBary;

out vec4 diffuse;
out vec4 lightColour;
out vec2 vBary;
out float distanceFromCamera;
out vec4 gl_Position;

void main() {
	// NOTE: This is per-vertex lighting. It's faster, but doesn't look as good
	// as per-pixel lighting. However, since the gears have no smooth faces,
	// per-pixel lighting is really not necessary.
	vec3 vPos = (model * vec4(aPos, 1.)).xyz;
	vec3 lightDiff = normalize(lightPos - vPos);
	mat3 rotation = mat3(model[0][0], model[0][1], model[0][2], model[1][0], model[1][1], model[1][2], model[2][0], model[2][1], model[2][2]);
	// vNrm = rotation * aNrm;
	vec3 vNrm = rotation * aNrm;
	float lightIntensity = max(0, dot(lightDiff, vNrm));
	diffuse = vec4(colour, 1.);
	lightColour = vec4(vec3(lightIntensity), 1.);
	vec4 screenPos = projection * view * model * vec4(aPos, 1.);
	distanceFromCamera = screenPos.z;
	vBary = aBary.yz;
	screenPos.w *= zoom;
	gl_Position = screenPos;
	// gl_Position = vec4(aPos, zoom);
}