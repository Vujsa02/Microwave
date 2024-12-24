#version 330 core

uniform int uBlinkState; // Blink state (1 = red, 0 = black)

out vec4 FragColor;

void main() {
    FragColor = (uBlinkState == 1) ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(0.0, 0.0, 0.0, 1.0); // Red or black

}
