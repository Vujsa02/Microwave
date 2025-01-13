#version 330 core

layout(location = 0) in vec3 inPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 uM; //Matrica transformacije
uniform mat4 uV; //Matrica kamere
uniform mat4 uP; //Matrica projekcija

out vec3 FragPos;  
out vec3 Normal;
out vec3 chCol;
out vec2 TexCoords;

void main()
{
	gl_Position = uP * uV * uM * vec4(inPos, 1.0); //Zbog nekomutativnosti mnozenja matrica, moramo mnoziti MVP matrice i tjemena "unazad"
	FragPos = vec3(uM * vec4(inPos, 1.0));
	Normal = mat3(transpose(inverse(uM))) * aNormal;  
	TexCoords = aTexCoords;
}