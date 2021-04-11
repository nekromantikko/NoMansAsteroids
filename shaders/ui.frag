#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 4) uniform sampler2D _texture;

layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_color;

layout(location = 0) out vec4 outColor;

void main() {
	vec2 texCoord = v_uv;
	vec4 texColor = texture(_texture, texCoord);
	vec4 finalColor = texColor * v_color;
	
	outColor = vec4(finalColor.rgb * finalColor.a, finalColor.a);
}