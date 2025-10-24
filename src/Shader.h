#ifndef SHADER_H_
#define SHADER_H_

#include <glad/glad.h>
#include <fstream>
#include <string>
#include <cerrno>

class Shader
{
public:
    GLuint ID;
    
    Shader() = default;
    Shader(const char* vertexFile, const char* fragmentFile, const char* fallbackVertexFile = nullptr, const char* fallbackFragmentFile = nullptr);
    void activateProgram();
    void deleteProgram();
};

#endif