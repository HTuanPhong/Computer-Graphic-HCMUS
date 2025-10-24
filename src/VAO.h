#ifndef VAO_CLASS_H
#define VAO_CLASS_H

#include <glad/glad.h>
#include "VBO.h"

class VAO
{
public:
	// ID reference for the Vertex Array Object
	GLuint ID;

	// Constructor that generates a VAO ID
	VAO();

	void linkVBO(VBO& VBO, GLuint layout);
	void bind();
	void unbind();
	void deleteObject();
};

#endif