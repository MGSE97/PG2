#pragma once
#include "vector3.h"
#include "matrix4x4.h"
#include "mymath.h"
#include "Transform.h"
#include "glutils.h"
#include <glad\glad.h>

// Not used, may be usefull in future
class Transform
{
public:
	Transform(const char* name);
	Transform* Use(GLuint* shader);
	Transform* Position(Vector3 position, bool set = false);
	Transform* Rotation(Vector3 rotation, bool set = false);
	Transform* Scale(Vector3 scale, bool set = false);
	Transform* Position(int x, int y, int z, bool set = false);
	Transform* Rotation(int x, int y, int z, bool set = false);
	Transform* Scale(int x, int y, int z, bool set = false);
	Transform* Scale(int scale, bool set = false);
	Transform* Position(float x, float y, float z, bool set = false);
	Transform* Rotation(float x, float y, float z, bool set = false);
	Transform* Scale(float x, float y, float z, bool set = false);
	Transform* Scale(float scale, bool set = false);
	Transform* Position(double x, double y, double z, bool set = false);
	Transform* Rotation(double x, double y, double z, bool set = false);
	Transform* Scale(double x, double y, double z, bool set = false);
	Vector3* GetPosition();
	Vector3* GetRotation();
	Vector3* GetScale();
private:
	Vector3 position_;
	Vector3 rotation_;
	Vector3 scale_;
	Matrix4x4 model_;
	bool changed_;
	const char* name_;
};

