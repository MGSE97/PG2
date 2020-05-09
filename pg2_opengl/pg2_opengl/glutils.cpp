#include "pch.h"
#include "glutils.h"

void SetMatrix4x4( const GLuint program, const GLfloat * data, const char * matrix_name )
{	
	const GLint location = glGetUniformLocation( program, matrix_name );

	if ( location == -1 )
	{
		if(VERBOSE)
			printf( "Matrix '%s' not found in active shader.\n", matrix_name );
	}
	else
	{
		glUniformMatrix4fv( location, 1, GL_TRUE, data );
	}
}

void SetVector3(const GLuint program, const GLfloat* data, const char* vector_name)
{
	const GLint location = glGetUniformLocation(program, vector_name);

	if (location == -1)
	{
		if (VERBOSE)
			printf("Vector '%s' not found in active shader.\n", vector_name);
	}
	else
	{
		glUniform3fv(location, 1, data);
	}
}

void SetSampler(const GLuint program, GLenum texture_unit, const char* sampler_name) 
{
	const GLint location = glGetUniformLocation(program, sampler_name);
	if (location == -1) 
	{
		if (VERBOSE)
			printf("Texture sampler '%s' not found in active shader.\n", sampler_name);
	}
	else 
	{
		glUniform1i(location, texture_unit);
	}
}

void SetBoolean(const GLuint program, GLboolean value, const char* sampler_name)
{
	const GLint location = glGetUniformLocation(program, sampler_name);
	if (location == -1)
	{
		if (VERBOSE)
			printf("Boolean '%s' not found in active shader.\n", sampler_name);
	}
	else
	{
		glUniform1i(location, (GLint)value);
	}
}
