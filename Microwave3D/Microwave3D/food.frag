#version 330 core
out vec4 FragColor;

in vec2 TexCoord;  // Texture coordinate passed from the vertex shader

uniform sampler2D uTex;  // Texture

void main() {
    // Fetch the color from the texture
    FragColor = texture(uTex, TexCoord);
}
