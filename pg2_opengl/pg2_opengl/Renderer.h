#pragma once
#include "camera.h"
#include "utils.h"
#include "objloader.h"
#include "Model.h"
#include "light.h"

enum TextureType {BRDF_Integration_Map, PreFiltered_Enviroment_Map, Irradiance_Map};

class Renderer
{
public:
	Renderer(const int width, const int height, const float fov_y, const Vector3 view_from, const Vector3 view_at, const Vector3 light_pos, std::string shader);
	~Renderer();

	void Prepare();
	void LoadScene(const char* file_name);
	void Update();
	void FinishSetup();

	void LoadTexture(const char* file, TextureType type);
	void LoadTextures(std::vector<const char*> files, TextureType type);

	void Draw();

	Camera camera;
	Light light;
	Model* model;
	GLFWwindow* window;

	void InitShadowDepthbuffer(); // initialize shadow (depth) map texture and framebuffer for the first pass
	void InitSSAODepthbuffer(); // initialize camera depth map texture and framebuffer
private:
	int InitGL();
	
	GLuint shader_program_;
	GLuint vertex_shader_;
	GLuint fragment_shader_;
	void PrepareShaders(GLuint* program, GLuint* vertex, GLuint* fragment, std::string shader);

	GLuint shadow_shader_program_;
	GLuint shadow_vertex_shader_;
	GLuint shadow_fragment_shader_;

	GLint CheckShader(const GLenum shader, const char* name);
	GLint CheckProgram(const GLenum program);
	std::string* LoadShader(const char* file_name);

	Matrix4x4 mlp;
	int shadow_width_{ 4096 }; // shadow map resolution
	int shadow_height_{ shadow_width_ };
	bool use_shadows_{ false };	// initialized shadow buffer
	GLuint fbo_shadow_map_{ 0 }; // shadow mapping FBO
	GLuint tex_shadow_map_{ 0 }; // shadow map texture
	GLuint handle_shadow_map_{ 0 };

	bool use_ssao_{ false };	// initialized shadow buffer
	GLuint fbo_ssao_map_{ 0 }; // ao mapping FBO
	GLuint tex_ssao_map_{ 0 }; // ao map texture
	GLuint tex_ssao_noise_{ 0 }; // ao noise texture
	GLuint handle_ssao_map_{ 0 }; // ao map texture
	GLuint handle_ssao_noise_{ 0 }; // ao noise texture

	GLuint tex_brdf_map_{ 0 };
	GLuint tex_env_map_{ 0 };
	GLuint tex_ir_map_{ 0 };
	GLuint handle_brdf_map_{ 0 };
	GLuint handle_env_map_{ 0 };
	GLuint handle_ir_map_{ 0 };

	void PrepareSSAO();
	void PrepareShadows();

	void UpdateSSAO();
	void UpdateShadows();

	void DrawShadows();
};

