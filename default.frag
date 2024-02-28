#version 330 core

uniform bool lit;
uniform bool wireframe;

// Calculated in the vertex shader
in vec4 lightColour;
in vec4 diffuse;
in vec2 vBary;
in float distanceFromCamera;

out vec4 FragColor;

// https://github.com/rreusser/glsl-solid-wireframe/blob/d7f98148133fb1357cf031812601dae368392db6/barycentric/scaled.glsl
// Copyright Ricky Reusser 2016. MIT License.
float gridFactor (vec2 vBC, float width, float feather) {
	float w1 = width - feather * 0.5;
	vec3 bary = vec3(vBC.x, vBC.y, 1.0 - vBC.x - vBC.y);
	vec3 d = fwidth(bary);
	vec3 a3 = smoothstep(d * w1, d * (w1 + feather), bary);
	return min(min(a3.x, a3.y), a3.z);
}

void main() {
	vec4 grayShade = vec4(vec3(distanceFromCamera / 50.) + .25, 1.);
	if (!wireframe) {
		FragColor = mix(grayShade, lightColour, float(lit)) * diffuse;
	} else {
		FragColor.rg = vBary;
		// FragColor.rgb = (1.0 - gridFactor(vBary, 0.5, 0.5)) * float(wireframe);
	}
}