#version 430 core

struct Particle {
    vec3 position;
    vec3 velocity;
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aNormal;
// layout (location = 2) in vec2 aTex;

// out vec3 Normal;

uniform mat4 model;
uniform mat4 camMatrix;

void main() {

    // Normal = -normalize(mat3(transpose(inverse(model)) * aNormal);

    uint id = gl_InstanceID;
    vec3 particlePosition = particles[id].position;
    gl_Position = camMatrix * model * vec4(aPos + particlePosition, 1.0);
}
