#version 450 core
layout (location = 0) in vec3 position;

// uniform variables
// Projection (P_l)*Light (V_l)*Model (M) matrix
uniform mat4 mlp;

void main( void )
{
	gl_Position = mlp * vec4(position, 1);
}
