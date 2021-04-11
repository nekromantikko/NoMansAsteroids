#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform GlobalMatrices
{
    mat4 view;
    mat4 proj;
	vec3 camPos;
} globalMatrices;

layout(binding = 3) uniform ShaderData
{
    vec4 color[2];
} shaderData;

layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_lightSpacePos;
layout(location = 2) in vec3 v_normal;
layout(location = 3) in vec3 v_worldPos;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 camDir = normalize(globalMatrices.camPos - v_worldPos);
	float a = abs(dot(camDir, v_normal));
	vec4 mixColor = mix(shaderData.color[0], shaderData.color[1], a);
	outColor = mixColor;
}