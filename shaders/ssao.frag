#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 4) uniform sampler2D _texture;
layout(binding = 5) uniform sampler2D _depth;

layout(binding = 0) uniform GlobalMatrices
{
    mat4 view;
    mat4 proj;
	vec3 camPos;
} globalMatrices;

layout(location = 0) out vec4 outColor;

void main() 
{

	float random[4][4] = {
	{5.27089, 0.907571, 3.12414, 1.71042},
	{6.23083, 0.575959, 3.24631, 1.36136},
	{2.84489, 2.63545, 0.663225, 4.64258},
	{1.98968, 5.044, 4.66003, 0.226893}
	};
	
	vec2 texCoord = vec2(gl_FragCoord.x,gl_FragCoord.y);
	
	//http://theorangeduck.com/page/pure-depth-ssao
	const int samples = 16;
	vec3 sample_sphere[samples] = {
	  vec3( 0.5381, 0.1856,-0.4319), vec3( 0.1379, 0.2486, 0.4430),
	  vec3( 0.3371, 0.5679,-0.0057), vec3(-0.6999,-0.0451,-0.0019),
	  vec3( 0.0689,-0.1598,-0.8547), vec3( 0.0560, 0.0069,-0.1843),
	  vec3(-0.0146, 0.1402, 0.0762), vec3( 0.0100,-0.1924,-0.0344),
	  vec3(-0.3577,-0.5301,-0.4358), vec3(-0.3169, 0.1063, 0.0158),
	  vec3( 0.0103,-0.5869, 0.0046), vec3(-0.0897,-0.4940, 0.3287),
	  vec3( 0.7119,-0.0154,-0.0918), vec3(-0.0533, 0.0596,-0.5411),
	  vec3( 0.0352,-0.0631, 0.5460), vec3(-0.4776, 0.2847,-0.0271)
	};
	
	float baseDepth = texture(_depth, texCoord).r;
	
	float falloff = 0.00005;
	
	float occlusion = 0.0;
	
	float increment = 1.0;
	float scale = 1.0;
	
	int modx = int(mod(texCoord.x, 4));
	int mody = int(mod(texCoord.y, 4));

	float angle = random[modx][mody];
	float cs = cos(angle);
	float sn = sin(angle);
	
	for (int i = 0; i < samples; i++)
	{
		vec2 s = sample_sphere[i].xy;
		vec2 rotatedSample = s;
		rotatedSample.x = s.x * cs - s.y * sn;
		rotatedSample.y = s.x * sn - s.y * cs;
	
		float depthSample = texture(_depth, rotatedSample * scale + texCoord).r;
		scale += increment;
		
		float diff = baseDepth - depthSample;
		float rangeCheck = 1.0 - smoothstep(0.0, falloff, diff);
		occlusion += diff * rangeCheck;
	}
	
	occlusion *= 20000;
	occlusion = clamp(occlusion, 0.0, 1.0);
	occlusion = 1.0 - occlusion;
	
	//outColor = vec4(occlusion, occlusion, occlusion, 1.0);
	
	vec3 texColor = texture(_texture, texCoord).rgb;
	outColor = vec4(texColor * occlusion, 1.0);
}