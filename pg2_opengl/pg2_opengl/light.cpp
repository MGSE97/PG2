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
