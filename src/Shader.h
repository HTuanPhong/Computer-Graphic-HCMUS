#ifndef SHADER_H_
#define SHADER_H_

#include <glad/glad.h>
#include <fstream>
#include <string>
#include <cerrno>

class Shader
{
public:
    // Reference ID of the Shader Program
    GLuint ID;
    
    // Constructor that build the Shader Program from 2 different shaders
    Shader(const char* vertexFile, const char* fragmentFile, const char* fallbackVertexFile = nullptr, const char* fallbackFragmentFile = nullptr);
    
    void activateProgram();
    void deleteProgram();
};

#endif