#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 app_pos;
layout(location = 1) in vec2 app_uv;
layout(location = 4) in vec4 app_color;

layout(binding = 0) uniform GlobalMatrices
{
    mat4 view;
    mat4 proj;
	vec3 camPos;
} globalMatrices;

layout(binding = 2) uniform PerInstanceData
{
	mat4 model;
} perInstanceData;

layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec4 v_color;

void main() {
    gl_Position = globalMatrices.proj * globalMatrices.view * perInstanceData.model * vec4(app_pos, 1.0);
    v_uv = app_uv;
	v_color = app_color;
}