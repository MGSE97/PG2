#version 450 core
layout (location = 0) in vec3 position;

// uniform variables
// Projection (P_l)*Light (V_l)*Model (M) matrix
uniform mat4 pv;
uniform mat4 mlp;

out vec3 position_lcs;

void main( void )
{
	gl_Position = pv * vec4(position, 1);
	vec4 tmp = mlp * vec4(position, 1.0);
	position_lcs = tmp.xyz / tmp.w;
}
