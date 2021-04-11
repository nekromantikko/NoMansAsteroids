#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 app_pos;

layout(binding = 0) uniform GlobalMatrices
{
    mat4 view;
    mat4 proj;
	vec3 camPos;
} globalMatrices;

layout(binding = 1) uniform LightingData
{
	mat4 mainLightMat;
	mat4 mainLightProjMat;
	vec4 mainLightColor;
} lightingData;

layout(binding = 2) uniform PerInstanceData
{
	mat4 model;
} perInstanceData;

void main() {
    gl_Position = lightingData.mainLightProjMat * lightingData.mainLightMat * perInstanceData.model * app_pos;
}