#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec2 texcoord;
layout (location = 4) in vec3 tangent;
layout (location = 5) in int materialIdx;

out vec2 tex;
out vec3 norm;
out mat3 TBN;
flat out int matIdx;

uniform mat4 pv; // View Projection

void main( void )
{
	gl_Position = pv * vec4(position, 1.0f); // model-space -> clip-space
	
	vec3 N = normal;
	vec3 T = tangent;
	vec3 B = cross(T, N);
	if(dot(cross(T, N), B) < 0)
		T = -T;
	TBN = mat3(T, B, N);
	
	tex =  vec2(texcoord.x, 1.0f - texcoord.y);
	norm = N;
	matIdx = materialIdx;
}
