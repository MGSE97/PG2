#include "pch.h"
#include "Renderer.h"

Renderer::Renderer(const int width, const int height, const float fov_y, const Vector3 view_from, const Vector3 view_at)
{
	Prepare();
	PrepareShaders();
	camera = Camera(width, height, fov_y, 0.8f, 1000.f, view_from, view_at, shader_program_);
}

Renderer::~Renderer()
{
	glDeleteShader(vertex_shader_);
	glDeleteShader(fragment_shader_);
	glDeleteProgram(shader_program_);

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

void Renderer::Draw()
{
	/*glUseProgram(shader_program_);
	camera.Update2();*/
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

/* invoked when window is resized */
void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
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

	glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
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
char* Renderer::LoadShader(const char* file_name)
{
	FILE* file = fopen(file_name, "rt");

	if (file == NULL)
	{
		printf("IO error: File '%s' not found.\n", file_name);

		return NULL;
	}

	size_t file_size = static_cast<size_t>(GetFileSize64(file_name));
	char* shader = NULL;

	if (file_size < 1)
	{
		printf("Shader error: File '%s' is empty.\n", file_name);
	}
	else
	{
		/* v glShaderSource nezadáváme v posledním parametru délku,
		takže øetìzec musí být null terminated, proto +1 a reset na 0*/
		shader = new char[file_size + 1];
		memset(shader, 0, sizeof(*shader) * (file_size + 1));

		size_t bytes = 0; // poèet již naètených bytù

		do
		{
			bytes += fread(shader, sizeof(char), file_size, file);
		} while (!feof(file) && (bytes < file_size));

		if (!feof(file) && (bytes != file_size))
		{
			printf("IO error: Unexpected end of file '%s' encountered.\n", file_name);
		}
	}

	fclose(file);
	file = NULL;

	return shader;
}

/* check shader for completeness */
GLint Renderer::CheckShader(const GLenum shader)
{
	GLint status = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	printf("Shader compilation %s.\n", (status == GL_TRUE) ? "was successful" : "FAILED");

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

void Renderer::PrepareShaders()
{
	std::string shader = "shaders/basic";
	//std::string shader = "shaders/lambert";
	//std::string shader = "shaders/normal";
	vertex_shader_ = glCreateShader(GL_VERTEX_SHADER);
	const char* vertex_shader_source = LoadShader((shader+"_shader.vert").c_str());
	glShaderSource(vertex_shader_, 1, &vertex_shader_source, nullptr);
	glCompileShader(vertex_shader_);
	SAFE_DELETE_ARRAY(vertex_shader_source);
	CheckShader(vertex_shader_);

	fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragment_shader_source = LoadShader((shader+"_shader.frag").c_str());
	glShaderSource(fragment_shader_, 1, &fragment_shader_source, nullptr);
	glCompileShader(fragment_shader_);
	SAFE_DELETE_ARRAY(fragment_shader_source);
	CheckShader(fragment_shader_);

	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertex_shader_);
	glAttachShader(shader_program_, fragment_shader_);
	glLinkProgram(shader_program_);
	CheckProgram(shader_program_);

	glUseProgram(shader_program_);
}
