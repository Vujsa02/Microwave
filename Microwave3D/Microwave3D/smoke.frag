in vec2 fragPos;
out vec4 FragColor;

uniform vec2 center;  // Center of the smoke ellipse
uniform vec2 originalCenter;
uniform vec2 scale;   // Scale for the ellipse (rx, ry)

void main() {
    vec2 scaledFragPos = (fragPos - center) / scale;
    float distance = length(scaledFragPos);

    // Compute opacity with an additional fade-out based on fragPos.y
    float fade = 1.0 - smoothstep(0, 2, fragPos.y - originalCenter.y); // Fade as Y increases
    float opacity = (1.0 - smoothstep(0, 1.0, distance)) * fade;

    FragColor = vec4(0.5, 0.5, 0.5, 0.8 * opacity);
}

