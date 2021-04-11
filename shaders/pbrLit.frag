#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 4) uniform sampler2D _texture;
layout(binding = 5) uniform sampler2D _normalMap;
layout(binding = 6) uniform sampler2D _occlusionMap;

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
	vec2 tiling;
	float metallic;
	float roughness;
	float ao;
} shaderData;

layout(location = 0) in vec2 v_uv;
layout(location = 1) in vec4 v_lightSpacePos;
layout(location = 2) in vec3 v_normal;
layout(location = 3) in vec3 v_worldPos;
layout(location = 4) in vec3 v_bitangent;
layout(location = 5) in vec3 v_tangent;

layout(location = 0) out vec4 outColor;

//Fresnel-Schlick approximation
vec3 fresnel_schlick(float cosTheta, vec3 reflectivity)
{
	return reflectivity + (1.0 - reflectivity) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnel_schlick_roughness(float cosTheta, vec3 reflectivity, float roughness)
{
	return reflectivity + (max(vec3(1.0 - roughness), reflectivity) - reflectivity) * pow(1.0 - cosTheta, 5.0);
}

const float PI = 3.14159265359;

//Trowbridge-Reitz GGX normal distribution
float distribution_ggx(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return num / denom;
}	

//Schlick-GGX geometry shadowing
float geometry_schlick_ggx(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	
	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}

//Smith's method
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N,V), 0.0);
	float NdotL = max(dot(N,L), 0.0);
	float ggx2 = geometry_schlick_ggx(NdotV, roughness);
	float ggx1 = geometry_schlick_ggx(NdotL, roughness);
	
	return ggx1 * ggx2;
}

void main() 
{
	vec2 texCoord = v_uv * shaderData.tiling;
	
	vec3 albedo = texture(_texture, texCoord).rgb * shaderData.color.rgb;
	
	//NORMAL MAPPING
	
	vec3 normalColor = texture(_normalMap, texCoord).rgb;
	normalColor = normalColor * 2.0 - 1.0;
	normalColor.b = 1.0;
	normalColor = normalize(normalColor);
	mat3 TBN = mat3(v_tangent, v_bitangent, v_normal);
	vec3 finalNormal = normalize(TBN * normalColor);
	
	//PBR
	
	vec3 N = finalNormal;
	vec3 V = normalize(globalMatrices.camPos - v_worldPos);
	vec3 L = normalize(lightingData.mainLightDirection.rgb);
	vec3 H = normalize(V + L);
	
	vec3 radiance = lightingData.mainLightColor.rgb * PI;
	
	float ior = 0.04;
	vec3 reflectivity = vec3(ior);
	reflectivity = mix(reflectivity, albedo, shaderData.metallic);
	
	float cosTheta = max(dot(H, V), 0.0);
	vec3 F = fresnel_schlick(cosTheta, reflectivity);
	
	float NDF = distribution_ggx(N, H, shaderData.roughness);
	float G = geometry_smith(N, V, L, shaderData.roughness);
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	vec3 specular = numerator / max(denominator, 0.001);
	
	vec3 kS = fresnel_schlick_roughness(max(dot(N, V), 0.0), reflectivity, shaderData.roughness);
	vec3 kD = vec3(1.0) - kS;
	
	int cubemapMipCount = textureQueryLevels(_cubeMap);
	float irradianceMip = cubemapMipCount * 0.9;
	vec3 irradiance = textureLod(_cubeMap, N, irradianceMip).rgb;
	
	float reflectionMip = cubemapMipCount * shaderData.roughness;
	vec3 R = reflect(-V, N);
	vec3 reflection = textureLod(_cubeMap, R, reflectionMip).rgb * kS;
	
	vec3 diffuse = albedo * irradiance;
	float occlusion = texture(_occlusionMap, texCoord).r * shaderData.ao;
	vec3 ambientLight = (kD * diffuse + reflection) * occlusion;
	
	kD *= 1.0 - shaderData.metallic;
	
	float NdotL = max(dot(N, L), 0.0);        
    vec3 lighting = (kD * albedo / PI + specular) * radiance * NdotL;
	
	//SHADOWMAPPING
	
	vec3 projCoords = v_lightSpacePos.xyz / v_lightSpacePos.w;
	
	float bias = 0.00002;
	float slopeBias = (1.0 - max(dot(lightingData.mainLightDirection.xyz, N),0.0)) * bias + bias;
	
	projCoords.z -= slopeBias;
	projCoords.xy = projCoords.xy * 0.5 + 0.5;

	float shadow = texture(_shadowMap, projCoords).r;
	
	vec3 finalColor = ambientLight + lighting * shadow;
	
	outColor = vec4(finalColor, 1.0);
}