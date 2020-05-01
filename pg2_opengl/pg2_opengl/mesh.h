#pragma once
#include "material.h"
class Mesh
{
public:
	Material* material;
	int offset;
	int count;
	
	Mesh(int offset, int count, Material* material);
	void Draw(float* mvp);
};

