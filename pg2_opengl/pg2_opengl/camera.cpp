#include "pch.h"
#include "camera.h"
#include "glutils.h"

Camera::Camera( const int width, const int height, const float fov_y,
	const Vector3 view_from, const Vector3 view_at, GLuint shader_program)
{
	width_ = width;
	height_ = height;
	fov_y_ = fov_y;

	view_from_ = view_from;
	view_at_ = view_at;

	shader_program_ = shader_program;

	Update();
}

Camera::Camera(const int width, const int height, const float fov_y, const float near_y, const float far_y, const Vector3 view_from, const Vector3 view_at, GLuint shader_program)
{

	width_ = width;
	height_ = height;
	fov_y_ = fov_y;

	view_from_ = view_from;
	view_at_ = view_at;

	shader_program_ = shader_program;

	n_y_ = near_y;
	f_y_ = far_y;

	Update2();
}

Vector3 Camera::view_from() const
{
	return view_from_;
}

float Camera::focal_length() const
{
	return f_y_;
}

void Camera::set_fov_y( const float fov_y )
{
	assert( fov_y > 0.0 );

	fov_y_ = fov_y;
}

void Camera::Update()
{
	f_y_ = height_ / ( 2.0f * tanf( fov_y_ * 0.5f ) );

	Vector3 z_c = view_from_ - view_at_;
	z_c.Normalize();
	Vector3 x_c = up_.CrossProduct( z_c );
	x_c.Normalize();
	Vector3 y_c = z_c.CrossProduct( x_c );
	y_c.Normalize();
	MW = Matrix4x4(x_c, y_c, z_c, view_from_);

	SetMatrix4x4(shader_program_, MW.data(), "mvp");
}

void Camera::Update2()
{
	float aspect = width_ / height_;
	f_y_ = height_ / (2.0f * tanf(fov_y_ * 0.5f));

	Vector3 z_c = view_from_ - view_at_;
	z_c.Normalize();
	Vector3 x_c = up_.CrossProduct(z_c);
	x_c.Normalize();
	Vector3 y_c = z_c.CrossProduct(x_c);
	y_c.Normalize();
	MW = Matrix4x4(x_c, y_c, z_c, view_from_);

	double a = (f_y_ + n_y_) / (n_y_ - f_y_);
	double b = (2 * f_y_ * n_y_) / (n_y_ - f_y_);
	MM = Matrix4x4(
		n_y_, 0, 0, 0,
		0, n_y_, 0, 0,
		0, 0, a, b,
		0, 0, -1, 0);

	MN = Matrix4x4(
		2.0 / width_, 0, 0, 0,
		0, 2.0 / height_, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	);

	MP = MN * MM;

	SetMatrix4x4(shader_program_, (MW*MP).data(), "mvp");
}

void Camera::MoveForward( const float dt )
{
	Vector3 ds = view_at_ - view_from_;
	ds.Normalize();
	ds *= dt;

	view_from_ += ds;
	view_at_ += ds;
}
