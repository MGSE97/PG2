#version 450 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require // uint64_t

flat in int matIdx;

out vec4 FragColor;

struct Material {
	vec3 diffuse;
	vec3 specular;
	vec3 ambient;
	vec3 emission;
	
	float shininess;
	float roughness;
	float metallicness;
	float reflectivity;
	float ior;

    uint64_t tex_diffuse;
	uint64_t tex_normal;
	uint64_t tex_rma;
};

layout (std430, binding = 0) readonly buffer Materials {
	Material materials[];
};

void main( void )
{
	FragColor = vec4(materials[matIdx].diffuse.rgb, 1.f);
}
