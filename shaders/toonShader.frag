#version 450
#extension GL_ARB_separate_shader_objects : enable

//https://stackoverflow.com/questions/16069959/glsl-how-to-ensure-largest-possible-float-value-without-overflow
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38

layout(binding = 4) uniform sampler2D _texture;
layout(binding = 5) uniform sampler2D _tintTexture;

layout(binding = 12) uniform sampler2DShadow _shadowMap;
layout(binding = 13) uniform samplerCube _cubeMap;

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
	vec4 mainLightDirection;
	vec4 ambientColor;
} lightingData;

layout(binding = 3) uniform ShaderData
{
    vec4 color;
	vec4 shadowTint;
	vec4 envColor;
	float shadowThreshold;
	float shadowSmoothness;
	vec4 specularColor;
	float specularThreshold;
	float specularSmoothness;
	float anisotropy;
	
} shaderData;

layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec3 v_normalWS;
layout(location = 2) in vec3 v_tangentWS;
layout(location = 3) in vec3 v_bitangentWS;
layout(location = 4) in vec3 v_viewDirWS;
layout(location = 5) in vec4 v_lightSpacePos;
layout(location = 6) in vec3 v_positionWS;
layout(location = 7) in float v_shadowThreshold;

layout(location = 0) out vec4 outColor;

vec3 SafeNormalize(vec3 inVec)
{
	float dp3 = max(FLT_MIN, dot(inVec, inVec));
	return inVec * inversesqrt(dp3);
}

void main()
{
	vec2 uv = v_uv;
	uv.y = 1.0 - uv.y; //must flip uvs

	//float shadowThreshold = 1.0 - v_shadowThreshold;
	float shadowThreshold = shaderData.shadowThreshold;

	vec3 T = normalize(v_tangentWS);
	vec3 B = normalize(v_bitangentWS);
	vec3 N = normalize(v_normalWS);

	vec3 L = normalize(lightingData.mainLightDirection.rgb);
	vec3 V = SafeNormalize(v_viewDirWS);
	//offset halway direction according to shadow threshold
	vec3 H = SafeNormalize(L + V * (1.0 - shadowThreshold));

	float NoV = dot(N, V);
	float NoL = dot(N, L);
	float NoH = dot(N, H);

	float ToH = dot(T, H);
	float BoH = dot(B, H);

	vec3 specular = vec3(0.0); //come up with better specular
	float cock = (1.0 - abs(BoH)) * shaderData.anisotropy + clamp(NoH, 0.0, 1.0) * (1.0 - shaderData.anisotropy);
	specular = vec3(smoothstep(shaderData.specularThreshold, shaderData.specularSmoothness + shaderData.specularThreshold, cock));
	specular *= shaderData.specularColor.rgb;

	//SHADOWMAPPING
	vec3 projCoords = v_lightSpacePos.xyz / v_lightSpacePos.w;

	float bias = 0.00002;
	float slopeBias = (1.0 - max(dot(lightingData.mainLightDirection.rgb, N),0.0)) * bias + bias;

	projCoords.z -= slopeBias;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;

	float shadowMap = texture(_shadowMap, projCoords).r;

	float diffuse = smoothstep(shadowThreshold, shaderData.shadowSmoothness + shadowThreshold, NoL);

	float shadowStrenght = clamp(NoL + 1.0, 0.0, 1.0);

	vec3 color = texture(_texture, uv).rgb * shaderData.color.rgb;

	vec3 tintColor = texture(_tintTexture, uv).rgb * shaderData.shadowTint.rgb;

	vec3 tint = mix(tintColor * 0.8, tintColor, shadowStrenght);

	float shadow = clamp(clamp(NoL, 0.0, 1.0) * shadowMap, 0.0, 1.0);
	vec3 shadowColor = mix(tint, vec3(1.0), vec3(diffuse * shadowMap));
	vec3 Lo = (color + specular * shadow) * shadowColor;

	float fresnel = pow((1.0 - NoV) * (1.0 - NoL), 2.0);

	vec3 env = fresnel * shaderData.envColor.rgb;

	vec3 finalColor = Lo + env;

	outColor = vec4(finalColor, 1.0);
}