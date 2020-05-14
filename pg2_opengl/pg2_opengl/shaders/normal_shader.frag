#version 450 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

uniform vec3 eye;

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

void main( void )
{
	vec3 normal = norm;
	if(materials[matIdx].tex_normal > 0)
		normal = normalize(TBN * normalize(texture(sampler2D(materials[matIdx].tex_normal), tex).rgb * 2.0 - 1.0));
	
	// Fix oposite normal
    if(dot(normal, eye) < 0)
        normal = -normal;

	FragColor = vec4(normal * 0.5 + 0.5, 1.0);
}
