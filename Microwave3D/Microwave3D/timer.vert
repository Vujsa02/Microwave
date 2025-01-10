#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 inCol;
out vec4 chCol;

uniform mat4 uM; //Matrica transformacije
uniform mat4 uV; //Matrica kamere
uniform mat4 uP; //Matrica projekcija

void main()
{
	gl_Position = uP * uV * uM * vec4(inPos, 1.0);
	chCol = inCol;
}