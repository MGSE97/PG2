#include "pch.h"
#include "Transform.h"

Transform::Transform(const char* name)
{
	position_ = Vector3(0, 0, 0);
	rotation_ = Vector3(0, 0, 0);
	scale_ = Vector3(1, 1, 1);
	changed_ = true;
	name_ = name;
}

Matrix4x4 rotate(Matrix4x4 a, float radians, Vector3 axis)
{
	float cosa = cos(radians), ncosa = 1 - cosa, sina = sin(radians);
	return Matrix4x4(
		ncosa * axis.x * axis.x + cosa,				ncosa * axis.y * axis.x - sina * axis.z,	ncosa * axis.z * axis.x + sina * axis.y, 0,
		ncosa * axis.x * axis.y + sina * axis.z,	ncosa * axis.y * axis.y + cosa,				ncosa * axis.z * axis.y - sina * axis.x, 0,
		ncosa * axis.x * axis.z - sina * axis.y,	ncosa * axis.y * axis.z + sina * axis.x,	ncosa * axis.z * axis.z + cosa, 0,
		0, 0, 0, 1
	) * a;
}

Matrix4x4 translate(Matrix4x4 a, Vector3 position)
{
	return Matrix4x4(
		position.x, 0, 0, 1,
		0, position.y, 0, 1,
		0, 0, position.z, 1,
		0, 0, 0, 1
	) * a;
}

Matrix4x4 scale(Matrix4x4 a, Vector3 scale)
{
	return Matrix4x4(
		1, 0, 0, scale.x,
		0, 1, 0, scale.y,
		0, 0, 1, scale.z,
		0, 0, 0, 1
	) * a;
}

Transform* Transform::Use(GLuint* shader)
{
	// Build new Model matrix
	if (changed_)
	{
		model_ =
			translate(Matrix4x4(1), position_) *
			rotate(Matrix4x4(1), deg2rad(rotation_.x), Vector3(1, 0, 0)) *
			rotate(Matrix4x4(1), deg2rad(rotation_.y), Vector3(0, 1, 0)) *
			rotate(Matrix4x4(1), deg2rad(rotation_.z), Vector3(0, 0, 1)) *
			scale(Matrix4x4(1), scale_);
	}

	SetMatrix4x4(*shader, model_.data(), name_);

	return this;
}

Transform* Transform::Position(Vector3 position, bool set)
{
	if (set)
		position_ = position;
	else
		position_ += position;
	changed_ = true;
	return this;
}

Transform* Transform::Rotation(Vector3 rotation, bool set)
{
	if (set)
		rotation_ = rotation;
	else
		rotation_ += rotation;
	changed_ = true;
	return this;
}

Transform* Transform::Scale(Vector3 scale, bool set)
{
	if (set)
		scale_ = scale;
	else
		scale_ += scale;
	changed_ = true;
	return this;
}

Transform* Transform::Position(int x, int y, int z, bool set)
{
	Position(Vector3(x, y, z), set);
	return this;
}

Transform* Transform::Rotation(int x, int y, int z, bool set)
{
	Rotation(Vector3(x, y, z), set);
	return this;
}

Transform* Transform::Scale(int x, int y, int z, bool set)
{
	Scale(Vector3(x, y, z), set);
	return this;
}

Transform* Transform::Scale(int scale, bool set)
{
	Scale(Vector3(static_cast<float>(scale)), set);
	return this;
}

Transform* Transform::Scale(float scale, bool set)
{
	Scale(Vector3(scale), set);
	return this;
}

Transform* Transform::Position(double x, double y, double z, bool set)
{
	Position(Vector3(x, y, z), set);
	return this;
}

Transform* Transform::Rotation(double x, double y, double z, bool set)
{
	Rotation(Vector3(x, y, z), set);
	return this;
}

Transform* Transform::Scale(double x, double y, double z, bool set)
{
	Scale(Vector3(x, y, z), set);
	return this;
}

Vector3* Transform::GetPosition()
{
	return &position_;
}

Vector3* Transform::GetRotation()
{
	return &rotation_;
}

Vector3* Transform::GetScale()
{
	return &scale_;
}

Transform* Transform::Position(float x, float y, float z, bool set)
{
	Position(Vector3(x, y, z), set);
	return this;
}

Transform* Transform::Rotation(float x, float y, float z, bool set)
{
	Rotation(Vector3(x, y, z), set);
	return this;
}

Transform* Transform::Scale(float x, float y, float z, bool set)
{
	Scale(Vector3(x, y, z), set);
	return this;
}
