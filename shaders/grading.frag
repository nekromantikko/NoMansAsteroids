#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 14) uniform sampler2D _texture;
layout(binding = 15) uniform sampler2D _depth;

layout(binding = 0) uniform GlobalMatrices
{
    mat4 view;
    mat4 proj;
	vec3 camPos;
} globalMatrices;

layout(location = 0) out vec4 outColor;

void main() 
{
	vec2 texCoord = vec2(gl_FragCoord.x,gl_FragCoord.y);
	
	float baseDepth = texture(_depth, texCoord).r;
	
	float f = fwidth(baseDepth);
	f *= 1000;
	f = 1.0 - f;
	
	outColor = vec4(f,f,f,1.0);
	outColor = texture(_texture, texCoord);
}