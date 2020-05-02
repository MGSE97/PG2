#include "pch.h"
#include "camera.h"
#include "glutils.h"
#include "mymath.h"

/*Camera::Camera( const int width_, const int height_, const float fov_y,
	const Vector3 view_from, const Vector3 view_at, GLuint shader_program)
{
	width = width_;
	height = height_;
	fov_y_ = fov_y;

	view_from_ = view_from;
	view_at_ = view_at;

	shader_program_ = shader_program;

	Update();
}*/

Camera::Camera(const int width_, const int height_, const float fov_x, const float near_y, const float far_y, const Vector3 view_from, const Vector3 view_at, GLuint shader_program)
{

	width = width_;
	height = height_;
	fov_x_ = fov_x;

	view_from_ = view_from;
	view_at_ = view_at;

	shader_program_ = shader_program;

	n_y_ = near_y;
	f_y_ = far_y;

	Update();
}

/*Vector3 Camera::view_from() const
{
	return view_from_;
}*/

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
	double a = (f_y_ + n_y_) / (n_y_ - f_y_);
	double b = (2.0 * f_y_ * n_y_) / (n_y_ - f_y_);

	double aspect = width / (float)height;
	double tan_fov_x = tan(fov_x_ / 2.0);
	MP = Matrix4x4(
		1.0 / tan_fov_x, 0, 0, 0,
		0, -aspect / tan_fov_x, 0, 0,
		0, 0, a, b,
		0, 0, -1, 0
	);

	Vector3 z_c = view_from_ - view_at_;
	z_c.Normalize();
	Vector3 x_c = up_.CrossProduct(z_c);
	x_c.Normalize();
	Vector3 y_c = z_c.CrossProduct(x_c);
	y_c.Normalize();
	MV = Matrix4x4(x_c, y_c, z_c, view_from_);
	MV.EuclideanInverse();

	// move to model
	Matrix4x4 mm(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
	
	SetMatrix4x4(shader_program_, (MP*MV*mm).data(), "pv");
	SetVector3(shader_program_, view_from_.data, "eye");
}

void Camera::MoveForward( const float dt )
{
	Vector3 ds = view_at_ - view_from_;
	ds.Normalize();
	ds *= dt;

	view_from_ += ds;
	view_at_ += ds;
}
