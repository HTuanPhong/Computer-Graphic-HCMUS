#include "Shader.hpp"
#include "App.hpp"
#include <stdio.h>

static char* loadFile(const char *path)
{
	FILE* file = fopen(path, "rb");
	if (file == NULL) { return NULL; }
	if (fseek(file, 0, SEEK_END) != 0) { //seek to end
		fclose(file);
		return NULL;
	}
	long size = ftell(file); //get size since we at end
	if (size <= 0) {
		fclose(file);
		return NULL;
	}
	if (fseek(file, 0, SEEK_SET) != 0) { //seek to begin
		fclose(file);
		return NULL;
	}

	char* content = (char*)malloc(size+1);
	size_t retSize = fread(content, (size_t)size, 1, file);
	if (retSize != 1) {
		fclose(file);
		return NULL;
	}
  content[size] = 0;
  fclose(file);
  return content;
}

bool CreateShaderProgram(const char *vertPath, const char *fragPath, GLuint *out_ID)
{
	GLint success;
	GLchar infoLog[1024];
	// Vertex Shader
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	char *vShaderCode = loadFile(vertPath);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 1024, NULL, infoLog);
		ErrLog("vertex shader failed to compile: %s", infoLog);
		return false;
	}
	// Fragment Shader
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	char *fShaderCode = loadFile(fragPath);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 1024, NULL, infoLog);
		ErrLog("fragment shader failed to compile: %s", infoLog);
		return false;
	}
	// Shader Program
	*out_ID = glCreateProgram();
	glAttachShader(*out_ID, vertex);
	glAttachShader(*out_ID, fragment);
	glLinkProgram(*out_ID);
	glGetProgramiv(*out_ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(*out_ID, 1024, NULL, infoLog);
		ErrLog("shader program failed to compile: %s", infoLog);
		return false;
	}
	// Clean up
	glDeleteShader(vertex);
	glDeleteShader(fragment);
  free(vShaderCode);
	free(fShaderCode);
	return true;
}
