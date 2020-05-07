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
	void PrepareShadows();

	void Draw();

	Camera camera;
	Light light;
	Model* model;
	GLFWwindow* window;

	void InitShadowDepthbuffer(); // initialize shadow (depth) map texture and framebuffer for the first pass
private:
	int InitGL();
	
	GLuint shader_program_;
	GLuint vertex_shader_;
	GLuint fragment_shader_;
	void PrepareShaders(GLuint* program, GLuint* vertex, GLuint* fragment, std::string shader);

	GLuint shadow_shader_program_;
	GLuint shadow_vertex_shader_;
	GLuint shadow_fragment_shader_;

	GLint CheckShader(const GLenum shader);
	GLint CheckProgram(const GLenum program);
	char* LoadShader(const char* file_name);

	Matrix4x4 mlp;
	int shadow_width_{ 1024 }; // shadow map resolution
	int shadow_height_{ shadow_width_ };
	bool use_shadows_{ false };	// initialized shadow buffer
	GLuint fbo_shadow_map_{ 0 }; // shadow mapping FBO
	GLuint tex_shadow_map_{ 0 }; // shadow map texture

	void DrawShadows();
};

