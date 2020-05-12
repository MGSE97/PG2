#include "pch.h"
#include "tutorials.h"
#include "utils.h"
#include "camera.h"
#include "mymath.h"
#include "Renderer.h"

#define AVANGER 0
#define CUBE 1
#define PIECE 2
#define COMANDER 3

Renderer* renderer;

/* invoked when window is resized */
void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	renderer->camera.width = width;
	renderer->camera.height = height;
	renderer->camera.Update();
}


#define SCENE PIECE
#define SCENE AVANGER
//#define SCENE COMANDER
//#define SCENE CUBE
int tutorial_2(const int width, const int height)
{
#if SCENE == AVANGER
	// Avanger
	renderer = new Renderer(width, height, deg2rad(45.0), Vector3(200, -150, 150), Vector3(0, 0, 35), Vector3(0, 0, 400), "shaders/pbr_shadow");
	//renderer = new Renderer(width, height, deg2rad(45.0), Vector3(200, -150, 150), Vector3(0, 0, 35), Vector3(0, 0, 400), "shaders/pbr");
	renderer->LoadScene("../../data/6887/6887_allied_avenger_gi2.obj");
	renderer->InitShadowDepthbuffer();
#elif SCENE == COMANDER
	// Commander (large)
	//renderer = new Renderer(width, height, deg2rad(45.0), Vector3(320, 330, 300), Vector3(0, 0, 30), Vector3(0, 0, 600), "shaders/pbr_shadow");
	renderer = new Renderer(width, height, deg2rad(45.0), Vector3(200, -150, 150), Vector3(0, 0, 35), Vector3(0, 0, 400), "shaders/pbr");
	renderer->LoadScene("../../data/6986/6986_mission_commander.obj");
#elif SCENE == CUBE
	// Cube
	renderer = new Renderer(width, height, deg2rad(45.0), Vector3(5, 5, 5), Vector3(0, 0, 0), Vector3(200, 200, 200), "shaders/pbr");
	renderer->LoadScene("../../data/cube/cube.obj");
#elif SCENE == PIECE
	// Piece
	//renderer = new Renderer(width, height, deg2rad(45.0), Vector3(25, -25, 40), Vector3(0, 0, 4), Vector3(0, 100, 50), "shaders/pbr_shadow");
	renderer = new Renderer(width, height, deg2rad(45.0), Vector3(10, -25, 15), Vector3(2, 2, 2), Vector3(52, 103, 53), "shaders/pbr");
	renderer->LoadScene("../../data/piece/piece_02.obj");
#endif

	renderer->LoadTexture("../../data/maps/brdf_integration_map_ct_ggx.exr", TextureType::BRDF_Integration_Map);
	renderer->LoadTexture("../../data/maps/lebombo_irradiance_map.exr", TextureType::Irradiance_Map);
	renderer->LoadTextures({
		"../../data/maps/lebombo_prefiltered_env_map_001_2048.exr",
		"../../data/maps/lebombo_prefiltered_env_map_010_1024.exr",
		"../../data/maps/lebombo_prefiltered_env_map_100_512.exr",
		"../../data/maps/lebombo_prefiltered_env_map_250_256.exr",
		"../../data/maps/lebombo_prefiltered_env_map_500_128.exr",
		"../../data/maps/lebombo_prefiltered_env_map_750_64.exr",
		"../../data/maps/lebombo_prefiltered_env_map_999_32.exr"
		}, TextureType::PreFiltered_Enviroment_Map);

	glfwSetFramebufferSizeCallback(renderer->window, framebuffer_resize_callback);
	
	renderer->FinishSetup();

	int i = 3;
	while (!glfwWindowShouldClose(renderer->window))
	{
		// Move camera
		if (glfwGetMouseButton(renderer->window, 2) == GLFW_PRESS)
		{
			if (glfwGetMouseButton(renderer->window, 0) == GLFW_PRESS)
			{
				i++;
				if (i > 8) i = 0;
				Sleep(1000);
			}
			else if (glfwGetMouseButton(renderer->window, 1) == GLFW_PRESS)
			{
				i--;
				if (i < 0) i = 8;
				Sleep(1000);
			}
		}
		if (glfwGetMouseButton(renderer->window, 0) == GLFW_PRESS)
		{
			if(i == 0)
				renderer->camera.view_at_.x++;
			if (i == 1)
				renderer->camera.view_at_.y++;
			if (i == 2)
				renderer->camera.view_at_.z++;
			if (i == 3)
				renderer->camera.view_from_.x++;
			if (i == 4)
				renderer->camera.view_from_.y++;
			if (i == 5)
				renderer->camera.view_from_.z++;

			if (i == 6)
				renderer->light.position_.x++;
			if (i == 7)
				renderer->light.position_.y++;
			if (i == 8)
				renderer->light.position_.z++;
		}
		if (glfwGetMouseButton(renderer->window, 1) == GLFW_PRESS)
		{

			if (i == 0)
				renderer->camera.view_at_.x--;
			if (i == 1)
				renderer->camera.view_at_.y--;
			if (i == 2)
				renderer->camera.view_at_.z--;
			if (i == 3)
				renderer->camera.view_from_.x--;
			if (i == 4)
				renderer->camera.view_from_.y--;
			if (i == 5)
				renderer->camera.view_from_.z--;

			if (i == 6)
				renderer->light.position_.x--;
			if (i == 7)
				renderer->light.position_.y--;
			if (i == 8)
				renderer->light.position_.z--;
		}
		printf("\r%d > %f %f %f, %f %f %f, %f %f %f", i, renderer->camera.view_at_.x, renderer->camera.view_at_.y, renderer->camera.view_at_.z, renderer->camera.view_from_.x, renderer->camera.view_from_.y, renderer->camera.view_from_.z, renderer->light.position_.x, renderer->light.position_.y, renderer->light.position_.z);
		renderer->camera.Update();
		renderer->light.Update();
		renderer->Update();

		// Drawing loop
		glClearColor( 0.2f, 0.3f, 0.3f, 1.0f ); // state setting function
		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ); // state using function
		renderer->Draw();
		glfwSwapBuffers(renderer->window);
		glfwPollEvents();
		Sleep(10);
	}
	delete renderer;
	return EXIT_SUCCESS;
}