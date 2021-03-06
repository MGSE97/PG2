#version 450 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec2 texcoord;
layout (location = 4) in vec3 tangent;
layout (location = 5) in int materialIdx;

out vec3 pos;
out vec2 tex;
out vec3 norm;
out mat3 TBN;
flat out int matIdx;

out vec3 light;
out vec3 eye;

uniform mat4 pv; // View Projection
uniform vec3 lightPos;
uniform vec3 eyePos;

void main( void )
{
	gl_Position = pv * vec4(position, 1.0f); // model-space -> clip-space

	//vec3 T = normalize(vec3(mat4(1) * vec4(tangent, 1.0)));
	//vec3 B = normalize(vec3(mat4(1) * vec4(cross(tangent, normals), 1.0)));
	//vec3 N = normalize(vec3(mat4(1) * vec4(normals, 1.0)));

	vec3 N = normal;
	vec3 T = tangent;
	vec3 B = cross(T, N);
	if(dot(cross(T, N), B) < 0)
		T = -T;
	TBN = mat3(T, B, N);

	pos = gl_Position.rgb;
	tex =  vec2(texcoord.x, 1.0f - texcoord.y);
	norm = N;
	matIdx = materialIdx;

	light = lightPos;
	eye = eyePos;
}
