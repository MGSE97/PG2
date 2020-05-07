#version 450 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

#define PI 3.14159265359
uniform vec3 light;
uniform vec3 eye;

in vec3 pos;
in vec2 tex;
in vec3 norm;
in mat3 TBN;
flat in int matIdx;

out vec4 FragColor;

layout (bindless_sampler) uniform;

struct Material {
	vec3 diffuse; // (1,1,1) or albedo
	uint64_t tex_diffuse; // albedo texture
	
	vec3 rma; // (1,1,1) or (roughness, metalness, 1)
	uint64_t tex_rma; // rma texture
	
	vec3 normal; // (1,1,1) or (0,0,1)
	uint64_t tex_normal; // bump texture
};

layout (std430, binding = 0) readonly buffer Materials {
	Material materials[];
};

float MNDF(in float a, in float NdH)
{
	float a2 = a*a;
	return a2/(PI*pow(NdH*NdH*(a2 - 1) + 1, 2));
}

float Fressnel(in float ior, in float NdH)
{
	float f0 = pow((1 - ior)/(1 + ior), 2);
	return f0 + (1 - f0) * pow(1 - NdH, 5);
}


void main( void )
{
	// Get texture and material data
	Material material = materials[matIdx];

	vec3 diffuse;
	if(material.tex_diffuse > 0)
		diffuse = texture(sampler2D(material.tex_diffuse), tex).rgb;
	else
		diffuse = material.diffuse.rgb;

	// Resolve bump map
	vec3 normal = norm;
	if(material.tex_normal > 0)
		normal = TBN * normalize(texture(sampler2D(material.tex_normal), tex).rgb * 2.0 - 1.0);

	// Resolve material params
	float ior = material.rma.b;

	vec3 rma = vec3(material.rma.r, material.rma.g, 1);
	if(material.tex_diffuse > 0)
		rma = texture(sampler2D(material.tex_rma), tex).rgb;

	float roughness = material.rma.r;
	float metallicness = material.rma.g;

	// Prepare vectors
	vec3 L = normalize(light - pos);
    vec3 V = normalize(-pos);
    vec3 H = normalize(L + V);

	// Prepare dotproducts
	float NdH = max(dot(normal, H), 0.001);

	// Compute PBR
	float ks = Fressnel(ior, NdH);


	FragColor = vec4(diffuse * max(dot(normal, light), 0), 1);
	//FragColor = vec4(normal * 0.5 + 0.5, 1);
}
