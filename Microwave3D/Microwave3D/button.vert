#version 330 core
layout(location = 0) in vec2 aPos; // Position attribute
layout(location = 1) in vec2 aTexCoord; // Texture coordinate attribute

out vec2 TexCoord; // Pass to fragment shader

void main() {
    gl_Position = vec4(aPos, 0.5, 1.0);
    TexCoord = aTexCoord;
}