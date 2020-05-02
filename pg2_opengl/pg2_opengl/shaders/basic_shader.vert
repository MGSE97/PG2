#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normals;
layout (location = 3) in vec2 texcoord;
layout (location = 5) in int materialIdx;

out vec2 tex;
out vec3 norm;
flat out int matIdx;

uniform mat4 mvp; // Model View Projection

void main( void )
{
	gl_Position = mvp * vec4(position, 1.0f); // model-space -> clip-space
	tex =  vec2(texcoord.x, 1.0f - texcoord.y);
	norm = normals;
	matIdx = materialIdx;
}
