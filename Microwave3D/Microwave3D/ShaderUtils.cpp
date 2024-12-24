#include "ShaderUtils.h"
#include <iostream>
#include <fstream>
#include <sstream>

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content;
    std::ifstream file(source);
    std::stringstream ss;

    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Successfully read file: \"" << source << "\"!" << std::endl;
    }
    else
    {
        std::cerr << "Error reading file: \"" << source << "\"!" << std::endl;
        return 0;
    }

    std::string temp = ss.str();
    const char* sourceCode = temp.c_str();

    unsigned int shader = glCreateShader(type);

    glShaderSource(shader, 1, &sourceCode, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
            << " shader compilation error: \n" << infoLog << std::endl;
        return 0;
    }

    return shader;
}

unsigned int createShader(const char* vsSource, const char* fsSource)
{
    unsigned int program = glCreateProgram();
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking error: \n" << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}


