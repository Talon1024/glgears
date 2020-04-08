#version 330 core

uniform vec3 lightPos;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNrm;
layout(location = 2) in vec3 aCol;

out vec4 diffuse;
out vec4 lightColour;
out vec3 vNrm;
// out float lightIntensity;

void main() {
	// NOTE: This is per-vertex lighting. It's faster, but doesn't look as good
	// as per-pixel lighting. However, since the gears have no smooth faces,
	// per-pixel lighting is really not necessary.
	vec3 vPos = (model * vec4(aPos, 1.)).xyz;
	mat3 rotation = mat3(model[0][0], model[0][1], model[0][2], model[1][0], model[1][1], model[1][2], model[2][0], model[2][1], model[2][2]);
	vNrm = rotation * aNrm;
	vec3 lightDiff = normalize(vPos - lightPos);
	float lightIntensity = dot(lightDiff, vNrm);
	diffuse = vec4(aCol, 1.);
	lightColour = vec4(aCol, 1.) * lightIntensity;
	gl_Position = projection * view * model * vec4(aPos, 1.);
	//gl_Position = vec4(aPos, 1.0);
}