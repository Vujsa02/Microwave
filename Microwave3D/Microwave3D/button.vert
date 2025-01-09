#version 330 core
layout(location = 0) in vec2 aPos; // Position attribute
layout(location = 1) in vec2 aTexCoord; // Texture coordinate attribute

uniform mat4 uM; //Matrica transformacije
uniform mat4 uV; //Matrica kamere
uniform mat4 uP; //Matrica projekcija

out vec2 TexCoord; // Pass to fragment shader

void main() {
	gl_Position = uP * uV * uM * vec4(aPos, -0.501, 1.0);
    TexCoord = aTexCoord;
}