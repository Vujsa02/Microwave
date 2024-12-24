#version 330 core

out vec4 FragColor;

// Uniform for the first rectangle color
uniform vec3 microwaveLightColor;
// Uniform to control transparency of the second rectangle
uniform float microwaveLightAlpha;

void main()
{
    // If the input color has non-zero alpha, use it directly
    // Otherwise, use the microwaveLightColor with microwaveLightAlpha
    FragColor = vec4(microwaveLightColor.rgb, microwaveLightAlpha);
    
}
