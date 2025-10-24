#ifndef VBO_H_
#define VBO_H_

#include <glad/glad.h>

class VBO
{
public:
	// Reference ID of the Vertex Buffer Object
	GLuint ID;
    
	// Constructor that generates a Vertex Buffer Object and links it to vertices
	VBO(GLfloat* vertices, GLsizeiptr size);

	void bind();
	void unbind();
	void deleteObject();
};

#endif