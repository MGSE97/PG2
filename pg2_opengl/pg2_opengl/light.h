#pragma once
#include "vector3.h"
#include "matrix4x4.h"
#include "camera.h"

class Light
{
public:
	Light() {};
	Light(Vector3 p, Vector3 l);
	void Set(Vector3 p, Vector3 l);
	void Use(GLuint program, const char* name_pos, const char* name_col);

	void SetShadows(Vector3 target, int width, int height, float near_, float far_, float fov_x, Vector3 up);
	void Update();

	int width_{ 640 }; // image width (px)
	int height_{ 480 };  // image height (px)

	float fov_x_{ 0.785f };
	float f_y_{ 1.0f }; // focal far lenght (px)
	float n_y_{ 0.0f }; // focal near lenght (px)
	Vector3 up_{ Vector3(0.0f, 0.0f, 1.0f) }; // up vector
	Vector3 position_;
	Vector3 color_;
	Vector3 target_;

	Matrix4x4 MLP;

	bool changed_;
private:

	Matrix4x4 MP; 
	Matrix4x4 MV;

	float fov_y_{ 0.785f }; // vertical field of view (rad)

};

