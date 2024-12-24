#version 330 core

layout(location = 0) in vec2 aPos; // Circle vertex positions
out vec2 fragPos;// Pass to fragment shader

uniform vec2 centerOffset;

void main() {
    vec2 transformedPos = aPos + centerOffset; // Apply offset
    fragPos = transformedPos; // Pass transformed position to fragment shader
    gl_Position = vec4(transformedPos, 0.0, 1.0); // Convert to clip space
}
