#pragma once
#include "camera.h"
#include "utils.h"
#include "objloader.h"
#include "Model.h"
#include "light.h"

class Renderer
{
public:
	Renderer(const int width, const int height, const float fov_y, const Vector3 view_from, const Vector3 view_at, const Vector3 light_pos);
	~Renderer();

	void Prepare();
	void LoadScene(const char* file_name);

	void Draw();

	Camera camera;
	Light light;
	Model* model;
	GLFWwindow* window;
private:
	int InitGL();
	
	GLuint shader_program_;
	GLuint vertex_shader_;
	GLuint fragment_shader_;
	void PrepareShaders();

	GLint CheckShader(const GLenum shader);
	GLint CheckProgram(const GLenum program);
	char* LoadShader(const char* file_name);
};

