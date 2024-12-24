#version 330 core
layout(location = 0) in vec2 aPos;  // Vertex position
layout(location = 1) in vec2 aTexCoord;  // Texture coordinate

out vec2 TexCoord;  // Pass to fragment shader

uniform float uAngle;  // Rotation angle (in radians)
uniform vec2 uCenter;  // Center of the rotation

void main() {
    // Pass the original position to the vertex shader (no change)
    gl_Position = vec4(aPos, 0.0, 1.0);

    // Apply the rotation to the texture coordinates (UV space)
    // Translate the texture coordinates to the origin
    vec2 translatedTexCoord = aTexCoord - 0.5;  // Centering the texture (0.5, 0.5 is the center of UV space)

    // Rotation matrix for texture coordinates
    mat2 rotationMatrix = mat2(cos(uAngle), -sin(uAngle), sin(uAngle), cos(uAngle));

    // Rotate the texture coordinates
    vec2 rotatedTexCoord = rotationMatrix * translatedTexCoord;

    // Translate the texture coordinates back to their original position
    TexCoord = rotatedTexCoord + 0.5;  // Return to original texture space
}
