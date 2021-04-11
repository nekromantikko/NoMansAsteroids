#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 app_pos;
layout(location = 1) in vec2 app_uv;
layout(location = 2) in vec3 app_normal;
layout(location = 3) in vec4 app_tangent;
layout(location = 4) in vec4 app_color;

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

layout(location = 0) out vec2 v_uv;
layout(location = 1) out vec3 v_normalWS;
layout(location = 2) out vec3 v_tangentWS;
layout(location = 3) out vec3 v_bitangentWS;
layout(location = 4) out vec3 v_viewDirWS;
layout(location = 5) out vec4 v_lightSpacePos;
layout(location = 6) out vec3 v_positionWS;
layout(location = 7) out float v_shadowThreshold;

void main() 
{
    v_uv = app_uv;
	
	mat4 normalMatrix = transpose(inverse(perInstanceData.model));
	v_normalWS = (normalMatrix * vec4(app_normal, 0.0)).xyz;
	v_tangentWS = (normalMatrix * vec4(app_tangent.xyz, 0.0)).xyz;
	vec3 bitangent = cross(app_normal, app_tangent.xyz) * app_tangent.w;
	v_bitangentWS	= (normalMatrix * vec4(bitangent, 0.0)).xyz;

	vec4 positionWS = perInstanceData.model * vec4(app_pos, 1.0);
	gl_Position = globalMatrices.proj * globalMatrices.view * positionWS;
	v_positionWS = positionWS.xyz;

	v_viewDirWS = globalMatrices.camPos - positionWS.xyz;

	v_shadowThreshold = app_color.r;

	v_lightSpacePos = lightingData.mainLightProjMat * lightingData.mainLightMat * vec4(v_positionWS, 1.0);
}