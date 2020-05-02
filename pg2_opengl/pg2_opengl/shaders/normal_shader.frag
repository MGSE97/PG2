#version 450 core

in vec3 norm;

out vec4 FragColor;

void main( void )
{
	FragColor = vec4(norm * 0.5 + 0.5, 1.0f);
}
