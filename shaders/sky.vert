#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 app_pos;

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

layout(location = 0) out vec3 v_uv;

void main() {
	vec4 position = globalMatrices.proj * vec4(app_pos, 1.0);
	gl_Position = position.xyww;
    v_uv = app_pos;
}