#version 450 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require

in vec2 tex;
in vec3 norm;
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

	//uvec2 tex_diffuse;
	//uvec2 tex_normal;
	//uvec2 tex_rma;
    uint64_t tex_diffuse;
	uint64_t tex_normal;
	uint64_t tex_rma;
};

layout (std430, binding = 0) readonly buffer Materials {
	Material materials[];
};

layout (bindless_sampler) uniform sampler2D bindless;

void main( void )
{
	if(materials[matIdx].tex_diffuse > 0)
		FragColor = texture(sampler2D(materials[matIdx].tex_diffuse), tex);
		//FragColor = texture(materials[matIdx].tex_diffuse, tex);
	else
		FragColor = vec4(materials[matIdx].diffuse.rgb, 1.f);
		
	//FragColor = texture(bindless, tex);
	//FragColor = vec4(tex.x, tex.y, 0.0f, 1.0f);
}
