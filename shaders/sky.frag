#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 3) uniform ShaderData
{
    vec4 color[2];
} shaderData;

layout(location = 0) in vec3 v_uv;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 mixColor = mix(shaderData.color[1], shaderData.color[0], v_uv.y * 0.5 + 0.5);
	outColor = mixColor;
}