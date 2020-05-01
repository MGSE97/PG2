#ifndef CAMERA_H_
#define CAMERA_H_

#include "vector3.h"
#include "matrix4x4.h"

/*! \class Camera
\brief A simple pin-hole camera.

\author Tomáš Fabián
\version 1.0
\date 2018-2019
*/
class Camera
{
public:
	Camera() { }

	/*Camera(const int width, const int height, const float fov_x,
		const Vector3 view_from, const Vector3 view_at, GLuint shader_program);*/
	
	Camera(const int width, const int height, const float fov_x,
		const float near_y, const float far_y,
		const Vector3 view_from, const Vector3 view_at, GLuint shader_program);


	Vector3 view_from_; // ray origin or eye or O
	Vector3 view_at_; // target T
	float focal_length() const;

	void set_fov_y(const float fov_y);

	void Update();
	void Update2();

	void MoveForward(const float dt);

	GLuint shader_program_;

	Matrix4x4 MW; // view matrix from CS -> WS	
	Matrix4x4 MP; // projection matrix from CS -> WS	
	Matrix4x4 MM; // M matrix from CS -> WS	
	Matrix4x4 MN; // N matrix from CS -> WS	

	int width{ 640 }; // image width (px)
	int height{ 480 };  // image height (px)
private:
	float fov_y_{ 0.785f }; // vertical field of view (rad)
	float fov_x_{ 0.785f };
	
	/*Vector3 view_from_; // ray origin or eye or O
	Vector3 view_at_; // target T*/
	Vector3 up_{ Vector3( 0.0f, 0.0f, 1.0f ) }; // up vector

	float f_y_{ 1.0f }; // focal far lenght (px)
	float n_y_{ 0.0f }; // focal near lenght (px)
};

#endif
