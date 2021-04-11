#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 4) uniform sampler2D _texture;
layout(binding = 12) uniform sampler2D _shadowMap;

layout(binding = 1) uniform LightingData
{
	mat4 mainLightMat;
	mat4 mainLightProjMat;
	vec4 mainLightColor;
} lightingData;

layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_lightSpacePos;
layout(location = 2) in vec3 v_normal;
layout(location = 3) in vec3 v_worldPos;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 lightDir = vec3(1.0, 1.0, 1.0);
	lightDir = normalize(lightDir);
	
	float ndotl = clamp(dot(v_normal, lightDir), 0.0, 1.0);
	
	vec3 projCoords = v_lightSpacePos.xyz / v_lightSpacePos.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(_shadowMap, projCoords.xy).r;
	closestDepth = closestDepth * 0.5 + 0.5;
	float currentDepth = projCoords.z;  
	float bias = 0.00001;
	float shadow = currentDepth - bias >= closestDepth ? 0.0 : 1.0; 
	
	vec4 ambientColor = {0.25,0.25,0.5,0.0};
	
	outColor = texture(_texture, v_uv) * (ndotl * lightingData.mainLightColor * shadow + ambientColor);
}