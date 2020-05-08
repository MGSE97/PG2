#include "pch.h"
#include "Renderer.h"
#include "glutils.h"


Renderer::Renderer(const int width, const int height, const float fov_x, const Vector3 view_from, const Vector3 view_at, const Vector3 light_pos)
{
	//std::string shader = "shaders/basic";
	//std::string shader = "shaders/pbr";
	std::string shader = "shaders/pbr_shadow";
	//std::string shader = "shaders/shadow_view";
	//std::string shader = "shaders/lambert";
	//std::string shader = "shaders/normal";

	Prepare();
	PrepareShaders(&shader_program_, &vertex_shader_, &fragment_shader_, shader);
	camera = Camera(width, height, fov_x, 0.8f, 10000.f, view_from, view_at, shader_program_);
	light = Light(light_pos);

	Vector3 up = { 0,0,1 };
	float lvdup = (light.position_ - view_at).DotProduct(up);
	if (lvdup == 0 || abs(lvdup) > 1)
		up = { 1, 0, 0 };
	light.SetShadows(view_at, shadow_width_, shadow_height_, 0.8f, 10000.f, fov_x, up);
	light.Update();
}

Renderer::~Renderer()
{
	glDeleteShader(vertex_shader_);
	glDeleteShader(fragment_shader_);
	glDeleteProgram(shader_program_);

	if (use_shadows_)
	{
		glDeleteShader(shadow_vertex_shader_);
		glDeleteShader(shadow_fragment_shader_);
		glDeleteProgram(shadow_shader_program_);
	}

	glfwTerminate();
}

void Renderer::Prepare()
{
	InitGL();
}

void Renderer::LoadScene(const char* file_name)
{
	std::vector<Surface*> surfaces;
	std::vector<Material*> materials;
	LoadOBJ(file_name, surfaces, materials);
	model = new Model(surfaces, materials);
}

void Renderer::UpdateShadows()
{
	if (use_shadows_ && light.changed_)
	{
		// --- first pass ---
		// set the shadow shader program and the viewport to match the size of the depth map
		glUseProgram(shadow_shader_program_);
		glViewport(0, 0, shadow_width_, shadow_height_);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow_map_);
		glClear(GL_DEPTH_BUFFER_BIT);
		
		// set up the light source through the MLP matrix
		SetMatrix4x4(shadow_shader_program_, light.MLP.data(), "mlp");
		
		// draw the scene
		model->Bind();

		// set back the main shader program and the viewport
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, camera.width, camera.height);
		glUseProgram(shader_program_);

		// Show as light
		return;
		camera.view_from_ = light.position_;
		camera.view_at_ = light.target_;
		camera.Update();
	}
}

void Renderer::Update()
{
	UpdateShadows();
	UpdateSSAO();
}

void Renderer::FinishSetup()
{
	PrepareSSAO();
}

float lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

void Renderer::PrepareSSAO()
{
	if (use_ssao_)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, camera.width, camera.height);
		glUseProgram(shader_program_);
		// Prepare samples
		std::uniform_real_distribution<float> d(0.0, 1.0);
		std::default_random_engine g;
		//std::vector<Vector3> samples;
		int sample_count = 64;
		for (unsigned int i = 0; i < sample_count; ++i)
		{
			Vector3 sample(
				d(g) * 2.0 - 1.0,
				d(g) * 2.0 - 1.0,
				d(g)
			);
			sample.Normalize();
			
			float scale = (float)i / (float)sample_count;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			
			//samples.push_back(sample);
			SetVector3(shader_program_, sample.data, (std::string("ssao_samples[") + std::to_string(i) + "]").c_str());
		}
		// Prepare noise
		std::vector<Vector3> noises;
		for (unsigned int i = 0; i < 16; i++)
		{
			Vector3 noise(
				d(g) * 2.0 - 1.0,
				d(g) * 2.0 - 1.0,
				0.0f);
			noises.push_back(noise);
		}
		glGenTextures(1, &tex_ssao_noise_);
		glBindTexture(GL_TEXTURE_2D, tex_ssao_noise_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &noises[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}

void Renderer::UpdateSSAO()
{
	if (use_ssao_ && light.changed_)
	{
		// --- first pass ---
		// set the shadow shader program and the viewport to match the size of the depth map
		glUseProgram(shadow_shader_program_);
		glViewport(0, 0, camera.width, camera.height);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_ssao_map_);
		glClear(GL_DEPTH_BUFFER_BIT);

		// set up the light source through the MLP matrix
		SetMatrix4x4(shadow_shader_program_, camera.MVP.data(), "mlp");

		// draw the scene
		model->Bind();

		// set back the main shader program and the viewport
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, camera.width, camera.height);
		glUseProgram(shader_program_);
	}
}

void Renderer::Draw()
{
	/*glUseProgram(shader_program_);
	camera.Update2();*/
	if (use_shadows_)
		DrawShadows();
	if (use_ssao_)
		DrawSSAO();
	light.Use(shader_program_, "light");
	model->Bind();
}


/* glfw callback */
void glfw_callback(const int error, const char* description)
{
	printf("GLFW Error (%d): %s\n", error, description);
}

/* OpenGL messaging callback */
void GLAPIENTRY gl_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
	printf("GL %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "Error" : "Message"),
		type, severity, message);
}


/* OpenGL check state */
bool check_gl(const GLenum error)
{
	if (error != GL_NO_ERROR)
	{
		//const GLubyte * error_str;
		//error_str = gluErrorString( error );
		//printf( "OpenGL error: %s\n", error_str );

		return false;
	}

	return true;
}

int Renderer::InitGL()
{

	glfwSetErrorCallback(glfw_callback);

	if (!glfwInit())
	{
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 8);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

	window = glfwCreateWindow(camera.width, camera.height, "PG2 OpenGL", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		if (!gladLoadGL())
		{
			return EXIT_FAILURE;
		}
	}

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(gl_callback, nullptr);

	printf("OpenGL %s, ", glGetString(GL_VERSION));
	printf("%s", glGetString(GL_RENDERER));
	printf(" (%s)\n", glGetString(GL_VENDOR));
	printf("GLSL %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	check_gl(NULL);

	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// map from the range of NDC coordinates <-1.0, 1.0>^2 to <0, width> x <0, height>
	glViewport(0, 0, camera.width, camera.height);
	// GL_LOWER_LEFT (OpenGL) or GL_UPPER_LEFT (DirectX, Windows) and GL_NEGATIVE_ONE_TO_ONE or GL_ZERO_TO_ONE
	glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
	
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}


/* load shader code from text file */
std::string* Renderer::LoadShader(const char* file_name)
{
	std::ifstream ifs(file_name);
	if(!ifs.is_open())
	{
		printf("IO error: File '%s' not found.\n", file_name);
		return NULL;
	}

	std::string* shader = new std::string((std::istreambuf_iterator<char>(ifs)),
										  (std::istreambuf_iterator<char>()));

	if (shader->empty())
	{
		printf("Shader error: File '%s' is empty.\n", file_name);
		return NULL;
	}
	//FILE* file = fopen(file_name, "rt");

	//if (file == NULL)
	//{
	//	printf("IO error: File '%s' not found.\n", file_name);

	//	return NULL;
	//}

	//size_t file_size = static_cast<size_t>(GetFileSize64(file_name));
	//char* shader = NULL;

	//if (file_size < 1)
	//{
	//	printf("Shader error: File '%s' is empty.\n", file_name);
	//}
	//else
	//{
	//	/* v glShaderSource nezadáváme v posledním parametru délku,
	//	takže øetìzec musí být null terminated, proto +1 a reset na 0*/
	//	shader = new char[file_size + 1];
	//	memset(shader, 0, sizeof(*shader) * (file_size + 1));

	//	size_t bytes = 0; // poèet již naètených bytù

	//	do
	//	{
	//		bytes += fread(shader, sizeof(char), file_size, file);
	//	} while (!feof(file) && (bytes < file_size));

	//	if (!feof(file) && (bytes != file_size))
	//	{
	//		printf("IO error: Unexpected end of file '%s' encountered.\n", file_name);
	//	}
	//}

	//fclose(file);
	//file = NULL;
	
	return shader;
}

void Renderer::DrawShadows()
{
	/*light.SetShadows(camera.view_at_, shadow_width_, shadow_height_, 0.8f, 1000.f, camera.fov_x_);
	light.Update();*/
	// everything is the same except this line
	SetMatrix4x4(shader_program_, light.MLP.data(), "mlp");
	// and also don't forget to set the sampler of the shadow map before entering rendering loop
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_shadow_map_);
	SetSampler(shader_program_, 0, "shadow_map");
}

void Renderer::DrawSSAO()
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex_ssao_map_);
	SetSampler(shader_program_, 1, "ssao_map");

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, tex_ssao_noise_);
	SetSampler(shader_program_, 2, "ssao_noise");
}

/* check shader for completeness */
GLint Renderer::CheckShader(const GLenum shader, const char* name)
{
	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	printf("Shader '%s' compilation %s.\n", name, (status == GL_TRUE) ? "was successful" : "FAILED");

	if (status == GL_FALSE)
	{
		int info_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_length);
		char* info_log = new char[info_length];
		memset(info_log, 0, sizeof(*info_log) * info_length);
		glGetShaderInfoLog(shader, info_length, &info_length, info_log);

		printf("Error log: %s\n", info_log);

		SAFE_DELETE_ARRAY(info_log);
	}

	return status;
}
/* check shader for completeness */
GLint Renderer::CheckProgram(const GLenum program)
{
	GLint status = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	printf("Program compilation %s.\n", (status == GL_TRUE) ? "was successful" : "FAILED");

	if (status == GL_FALSE)
	{
		int info_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_length);
		char* info_log = new char[info_length];
		memset(info_log, 0, sizeof(*info_log) * info_length);
		glGetProgramInfoLog(program, info_length, &info_length, info_log);

		printf("Error log: %s\n", info_log);

		SAFE_DELETE_ARRAY(info_log);
	}

	return status;
}

void Renderer::PrepareShaders(GLuint* program, GLuint* vertex, GLuint* fragment, std::string shader)
{
	*vertex = glCreateShader(GL_VERTEX_SHADER);
	std::string* vertex_shader_source = LoadShader((shader+"_shader.vert").c_str());
	const char* c_str = vertex_shader_source->c_str();
	glShaderSource(*vertex, 1, &c_str, nullptr);
	glCompileShader(*vertex);
	SAFE_DELETE(vertex_shader_source);
	CheckShader(*vertex, (shader + " vert").c_str());

	*fragment = glCreateShader(GL_FRAGMENT_SHADER);
	std::string* fragment_shader_source = LoadShader((shader+"_shader.frag").c_str());
	c_str = fragment_shader_source->c_str();
	glShaderSource(*fragment, 1, &c_str, nullptr);
	glCompileShader(*fragment);
	SAFE_DELETE(fragment_shader_source);
	CheckShader(*fragment, (shader + " frag").c_str());

	*program = glCreateProgram();
	glAttachShader(*program, *vertex);
	glAttachShader(*program, *fragment);
	glLinkProgram(*program);
	CheckProgram(*program);

	glUseProgram(*program);
}

void Renderer::InitShadowDepthbuffer()
{
	glGenTextures(1, &tex_shadow_map_); // texture to hold the depth values from the light's perspective
	glBindTexture(GL_TEXTURE_2D, tex_shadow_map_);
	// GL_DEPTH_COMPONENT ... each element is a single depth value. The GL converts it to floating point, multiplies by the signed scale
	// factor GL_DEPTH_SCALE, adds the signed bias GL_DEPTH_BIAS, and clamps to the range [0, 1] – this will be important later
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadow_width_, shadow_height_, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const float color[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // areas outside the light's frustum will be lit
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &fbo_shadow_map_); // new frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_shadow_map_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_shadow_map_, 0); // attach the texture as depth
	glDrawBuffer(GL_NONE); // we dont need any color buffer during the first pass
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the default framebuffer back

	use_shadows_ = true;
	PrepareShaders(&shadow_shader_program_, &shadow_vertex_shader_, &shadow_fragment_shader_, "shaders/shadow");
}

void Renderer::InitSSAODepthbuffer()
{
	glGenTextures(1, &tex_ssao_map_); // texture to hold the depth values from the light's perspective
	glBindTexture(GL_TEXTURE_2D, tex_ssao_map_);
	// GL_DEPTH_COMPONENT ... each element is a single depth value. The GL converts it to floating point, multiplies by the signed scale
	// factor GL_DEPTH_SCALE, adds the signed bias GL_DEPTH_BIAS, and clamps to the range [0, 1] – this will be important later

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, camera.width, camera.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const float color[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // areas outside the light's frustum will be lit
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &fbo_ssao_map_); // new frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_ssao_map_);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex_ssao_map_, 0); // attach the texture as depth
	glDrawBuffer(GL_NONE); // we dont need any color buffer during the first pass
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the default framebuffer back

	use_ssao_ = true;	// ToDo: use working solution https://learnopengl.com/Advanced-Lighting/SSAO
}