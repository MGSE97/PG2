#pragma once
#include <vector>
#include "material.h"
#include "surface.h"
#include <glad\glad.h>
#include "mesh.h"

class Model
{
public:
	Model(std::vector<Surface*> surfaces, std::vector<Material*> materials);
	~Model();
	
	void Bind();
private:
	GLuint vao_;
	GLuint vbo_;
	GLMaterial* materials_;
	std::vector<Mesh> meshes_;
	int triangle_count;
};

