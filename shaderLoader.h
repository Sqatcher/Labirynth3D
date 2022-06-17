#include "gl/glew.h"
#include "GL/freeglut.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char * shaderLoadSource(const char *filePath);
static GLuint shaderCompileFromFile(GLenum type, const char *filePath);
void shaderAttachFromFile(GLuint program, GLenum type, const char *filePath);
int loadShaders2(const char * vertexShaderPath, const char * fragmentShaderPath, const char* geometryShaderPath);
int loadShaders(const char * vertexShaderPath, const char * fragmentShaderPath);