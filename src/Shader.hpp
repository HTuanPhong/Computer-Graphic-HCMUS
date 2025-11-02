#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

bool CreateShaderProgram(const char *vertPath, const char *fragPath, GLuint *out_ID);

#endif