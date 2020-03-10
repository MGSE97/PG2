#version 450 core
in vec2 tex;

out vec4 FragColor;

float gold_noise(in vec2 xy, in float seed);

void main( void )
{
	//FragColor = vec4( 1.0f, 0.25f, 0.0f, 1.0f );
	//FragColor = vec4(noise1(1), tex, 1.0f );
	FragColor = vec4(gold_noise(tex, 500.f), tex, 1.f);
}

float PHI = 1.61803398874989484820459;

float gold_noise(in vec2 xy, in float seed) {
	return fract(tan(distance(xy*PHI, xy)*seed)*xy.x);
}
