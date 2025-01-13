#version 330 core

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;

struct Material {
    sampler2D frontDiffuse;  // Texture for the front side
    sampler2D backDiffuse;   // Texture for the back side
    vec3 specular;
    float shininess;
}; 
  
struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light; 
uniform Material material;

void main()
{    
    // Select the appropriate diffuse texture based on the side being rendered
    vec3 diffuseColor = vec3(texture(gl_FrontFacing ? material.frontDiffuse : material.backDiffuse, TexCoords));
    
    // ambient
    vec3 ambient = light.ambient * diffuseColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseColor;  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}