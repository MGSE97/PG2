#include "pch.h"
#include "mesh.h"

Mesh::Mesh(int offset, int count, Material* material)
{
	this->offset = offset;
	this->count = count;
	this->material = material;
}

void Mesh::Draw(float* mvp)
{
	//material->ActivateShader(mvp);
	glDrawArrays(GL_TRIANGLES, offset, count);
}
