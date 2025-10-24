#ifndef EBO_H_
#define EBO_H_

#include <glad/glad.h>

class EBO
{
public:
    // ID reference of Elements Buffer Object
    GLuint ID;

    // Constructor that generates a Elements Buffer Object and links it to indices
    EBO(GLuint* indices, GLsizeiptr size);

    void bind();
    void unbind();
    void deleteObject();
};

#endif