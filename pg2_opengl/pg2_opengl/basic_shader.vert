#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord;

out vec2 tex;

uniform mat4 mvp; // Model View Projection

void main( void )
{
	//gl_Position = vec4( position.x, -position.y, position.z, 1.0f );
	gl_Position = mvp * vec4(position.x, position.y, position.z, 1.0f ); // model-space -> clip-space
	tex = texcoord;
}
