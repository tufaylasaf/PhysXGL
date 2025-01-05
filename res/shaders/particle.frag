#version 430 core

out vec4 FragColor;

in vec3 Normal;

flat in uint particleID; // Pass particle ID to differentiate particles

uniform vec3 sunDirection;
uniform vec3 ambient;

// Function to generate a pseudo-random number based on particle ID
float random(uint id) {
    float res = fract(sin(float(id) * 12.9898 + 78.233) * 43758.5453);
    return res;
}

// Function to convert HSV to RGB
vec3 hsvToRgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1.0 - abs(mod(h * 6.0, 2.0) - 1.0));
    float m = v - c;

    vec3 rgb;
    if (h < 1.0 / 6.0) {
        rgb = vec3(c, x, 0);
    } else if (h < 2.0 / 6.0) {
        rgb = vec3(x, c, 0);
    } else if (h < 3.0 / 6.0) {
        rgb = vec3(0, c, x);
    } else if (h < 4.0 / 6.0) {
        rgb = vec3(0, x, c);
    } else if (h < 5.0 / 6.0) {
        rgb = vec3(x, 0, c);
    } else {
        rgb = vec3(c, 0, x);
    }

    return rgb + m;
}

void main() {
    // Generate a random hue based on particle ID
    float hue = random(particleID);

    // Convert random hue to RGB with full saturation and value
    vec3 randomColor = hsvToRgb(hue, 1.0, 1.0);
    randomColor = vec3(1.0);

    // Apply lighting
    float f = max(dot(-sunDirection, normalize(Normal)), 0.0);
    vec3 c = ambient + randomColor * f;

    // Gamma correction
    c = pow(c, vec3(1.0 / 2.2));

    FragColor = vec4(c, 1.0);
}
