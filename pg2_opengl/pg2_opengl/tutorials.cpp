#include "pch.h"
#include "tutorials.h"
#include "utils.h"
#include "camera.h"
#include "mymath.h"
#include "Renderer.h"

#define AVANGER 0
#define CUBE 1
#define PIECE 2

Renderer* renderer;

/* invoked when window is resized */
void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	renderer->camera.width = width;
	renderer->camera.height = height;
	renderer->camera.Update();
}

//#define SCENE PIECE
#define SCENE AVANGER
int tutorial_2(const int width, const int height)
{
#if SCENE == AVANGER
	// Avanger
	renderer = new Renderer(width, height, deg2rad(45.0), Vector3(200, -150, 150), Vector3(0, 0, 35), Vector3(10, 0, 400));
	renderer->LoadScene("../../data/6887/6887_allied_avenger_gi2.obj");
#elif SCENE == CUBE
	// Cube
	renderer = new Renderer(width, height, deg2rad(45.0), Vector3(5, 5, 5), Vector3(0, 0, 0), Vector3(200, 200, 200));
	renderer->LoadScene("../../data/cube/cube.obj");
#elif SCENE == PIECE
	// Piece
	renderer = new Renderer(width, height, deg2rad(45.0), Vector3(25, -25, 40), Vector3(0, 0, 4), Vector3(200, 200, 200));
	renderer->LoadScene("../../data/piece/piece_02.obj");
#endif

	renderer->InitShadowDepthbuffer();
	renderer->PrepareShadows();

	glfwSetFramebufferSizeCallback(renderer->window, framebuffer_resize_callback);

	int i = 0;
	while (!glfwWindowShouldClose(renderer->window))
	{
		// Move camera
		if (glfwGetMouseButton(renderer->window, 2) == GLFW_PRESS)
		{
			i++;
			if (i >= 6) i = 0;
			Sleep(1000);
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
		}
		printf("\r%d > %f %f %f, %f %f %f", i, renderer->camera.view_at_.x, renderer->camera.view_at_.y, renderer->camera.view_at_.z, renderer->camera.view_from_.x, renderer->camera.view_from_.y, renderer->camera.view_from_.z);
		renderer->camera.Update();

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