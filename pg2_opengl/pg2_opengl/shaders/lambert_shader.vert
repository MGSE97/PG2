#version 450 core
layout (location = 0) in vec3 position;
layout (location = 5) in int materialIdx;

flat out int matIdx;

uniform mat4 mvp; // Model View Projection

void main( void )
{
	gl_Position = mvp * vec4(position, 1.0f); // model-space -> clip-space
	matIdx = materialIdx;
}
