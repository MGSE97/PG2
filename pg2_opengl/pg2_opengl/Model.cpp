#include "pch.h"
#include "Model.h"

int set_attribute(int stride, int location, GLenum type, int size, int offset)
{
	if (type == GL_FLOAT)
		glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset);
	else
		glVertexAttribIPointer(location, size, GL_INT, stride, (GLvoid*)offset);
	glEnableVertexAttribArray(location);
	return size * (type == GL_FLOAT? sizeof(GLfloat) : sizeof(GLint));
}

Model::Model(std::vector<Surface*> surfaces, std::vector<Material*> materials)
{
	// Prepare vertex buffer
	triangle_count = 0;
	for (const Surface* s : surfaces) 
		triangle_count += s->triangles_count();

	Triangle* triangles = new Triangle[triangle_count];

	int mIdx = 0;
	for (Material* mat : materials)
		mat->setIndex(mIdx++);

	int offset_triangles = 0;
	int offset_vertices = 0;
	for (Surface* s : surfaces)
	{
		offset_triangles += s->CopyTriangles(triangles, offset_triangles);
		meshes_.push_back(Mesh(offset_vertices, s->no_vertices(), s->get_material()));
		offset_vertices += s->no_vertices();
	}

	// Create VBO, VAO
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	/*float* arr = (float*)triangles;
	for (int i = 0; i < offset_vertices; i++)
		printf("%f %f %f, %f %f %f, %f %f %f, %f %f, %f %f %f, %d\n", 
			arr[i * sizeof(Vertex)/sizeof(float)], 
			arr[i * sizeof(Vertex) / sizeof(float) + 1],
			arr[i * sizeof(Vertex) / sizeof(float) + 2],
			arr[i * sizeof(Vertex) / sizeof(float) + 3],
			arr[i * sizeof(Vertex) / sizeof(float) + 4],
			arr[i * sizeof(Vertex) / sizeof(float) + 5],
			arr[i * sizeof(Vertex) / sizeof(float) + 6],
			arr[i * sizeof(Vertex) / sizeof(float) + 7],
			arr[i * sizeof(Vertex) / sizeof(float) + 8],
			arr[i * sizeof(Vertex) / sizeof(float) + 9],
			arr[i * sizeof(Vertex) / sizeof(float) + 10],
			arr[i * sizeof(Vertex) / sizeof(float) + 11],
			arr[i * sizeof(Vertex) / sizeof(float) + 12],
			arr[i * sizeof(Vertex) / sizeof(float) + 13],
			arr[i * sizeof(Vertex) / sizeof(float) + 14]);*/
	glBufferData(GL_ARRAY_BUFFER, offset_vertices * sizeof(Vertex), triangles, GL_STATIC_DRAW);

	// vertex position
	int location = 0;
	int offset = 0;
	offset += set_attribute(sizeof(Vertex), location++, GL_FLOAT, 3, offset);
	// normal
	offset += set_attribute(sizeof(Vertex), location++, GL_FLOAT, 3, offset);
	// color
	offset += set_attribute(sizeof(Vertex), location++, GL_FLOAT, 3, offset);
	// texture coordinates
	offset += set_attribute(sizeof(Vertex), location++, GL_FLOAT, 2, offset);
	// tangent
	offset += set_attribute(sizeof(Vertex), location++, GL_FLOAT, 3, offset);
	// matid
	offset += set_attribute(sizeof(Vertex), location++, GL_INT, 1, offset);

	//upload materials 
	GLMaterial* glMaterials = new GLMaterial[materials.size()];
	int m = 0;
	for (auto& mat : materials) {
		glMaterials[m++] = mat->CreateStruct();
	}
	GLuint ssboMaterials = 0;
	glGenBuffers(1, &ssboMaterials);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboMaterials);
	const GLsizeiptr gl_materials_size = sizeof(GLMaterial) * materials.size();
	glBufferData(GL_SHADER_STORAGE_BUFFER, gl_materials_size, glMaterials, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboMaterials);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	delete[] glMaterials;
}

Model::~Model()
{
	glDeleteVertexArrays(1, &vao_);
	glDeleteBuffers(1, &vbo_);
}

void Model::Bind()
{
	//glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBindVertexArray(vao_);
	//glDrawArrays(GL_TRIANGLES, 0, triangle_count*3* sizeof(Vertex));
	for (const Mesh& mesh : meshes_)
	{
		glDrawArrays(GL_TRIANGLES, mesh.offset, mesh.count);
	}
}
