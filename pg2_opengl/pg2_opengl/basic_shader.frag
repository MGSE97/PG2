#version 450 core
in vec2 tex;
in vec3 norm;
flat in int matIdx;

out vec4 FragColor;

struct Material {
	vec3 diffuse;
	vec3 specular;
	vec3 ambient;
};

layout (std430, binding = 0) readonly buffer Materials {
	Material materials[];
};

float gold_noise(in vec2 xy, in float seed);

void main( void )
{
	//FragColor = vec4( 1.0f, 0.25f, 0.0f, 1.0f );
	//FragColor = vec4(noise1(1), tex, 1.0f );
	//FragColor = vec4(gold_noise(tex, 500.f), tex, 1.f);
	//FragColor = vec4(tex.x, tex.y, 0.0f, 1.0f);
	//FragColor = vec4(norm, 1.0f);
	FragColor = vec4(materials[matIdx].diffuse.rgb, 1.f);
}

float PHI = 1.61803398874989484820459;

float gold_noise(in vec2 xy, in float seed) {
	return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}
