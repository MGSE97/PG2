#include "pch.h"
#include "light.h"
#include "glutils.h"

Light::Light(float x, float y, float z)
{
	position_ = { x, y, z };
	changed_ = true;
}

Light::Light(Vector3 p)
{
	position_ = p;
	changed_ = true;
}

Vector3 Light::Get()
{
	return position_;
}

Vector3 Light::Set(float x, float y, float z)
{
	position_ = { x, y, z };
	changed_ = true;
	return position_;
}

Vector3 Light::Set(Vector3 p)
{
	position_ = p;
	changed_ = true;
	return position_;
}

void Light::Use(GLuint program, const char* name)
{
	if (changed_)
	{
		SetVector3(program, position_.data, name);
		changed_ = false;
	}
}

void Light::SetShadows(Vector3 target, int width, int height, float near_, float far_, float fov_x, Vector3 up)
{
	width_ = width;
	height_ = height;
	target_ = target;
	f_y_ = far_;
	n_y_ = near_;
	fov_x_ = fov_x;
	up_ = up;
}

void Light::Update()
{
	double a = (f_y_ + n_y_) / (n_y_ - f_y_);
	double b = (2.0 * f_y_ * n_y_) / (n_y_ - f_y_);

	double aspect = width_/height_;
	double tan_fov_x = tan(fov_x_ / 2.0);
	MP = Matrix4x4(
		1.0 / tan_fov_x, 0, 0, 0,
		0, -aspect / tan_fov_x, 0, 0,
		0, 0, a, b,
		0, 0, -1, 0
	);

	Vector3 z_c = position_ - target_;
	z_c.Normalize();
	Vector3 x_c = up_.CrossProduct(z_c);
	x_c.Normalize();
	Vector3 y_c = z_c.CrossProduct(x_c);
	y_c.Normalize();
	MV = Matrix4x4(x_c, y_c, z_c, position_);
	MV.EuclideanInverse();

	MLP = MP * MV;
	changed_ = true;
}
