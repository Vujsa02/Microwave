#pragma once
#ifndef SHADER_UTILS_H
#define SHADER_UTILS_H

#include <GL/glew.h>
#include <string>

// Compiles a shader of the given type (vertex or fragment) from source code
unsigned int compileShader(GLenum type, const char* source);

// Creates a shader program by linking vertex and fragment shaders
unsigned int createShader(const char* vsSource, const char* fsSource);


#endif // SHADER_UTILS_H
