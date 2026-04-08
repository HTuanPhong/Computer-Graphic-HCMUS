#ifndef SHADER_H
#define SHADER_H

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include <glad/glad.h>
#endif

bool CreateShaderProgram(const char *vertPath, const char *fragPath,
                         GLuint *out_ID);

#endif