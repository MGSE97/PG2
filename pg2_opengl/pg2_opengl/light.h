#pragma once
#include "vector3.h"

class Light
{
public:
	Light() {};
	Light(float x, float y, float z);
	Light(Vector3 p);
	Vector3 Get();
	Vector3 Set(float x, float y, float z);
	Vector3 Set(Vector3 p);
	void Use(GLuint program, const char* name);
private:
	Vector3 position_;
	bool changed_;
};

