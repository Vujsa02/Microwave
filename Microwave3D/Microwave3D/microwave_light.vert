#version 330 core

layout(location = 0) in vec2 aPos; // Circle vertex positions
out vec2 fragPos;                 // Pass to fragment shader

void main() {
    fragPos = aPos; // Pass vertex position to fragment shader
    gl_Position = vec4(aPos, 0.0, 1.0); // Convert to clip space
}