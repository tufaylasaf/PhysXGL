#version 430 core

struct Particle {
    vec3 pos;
    vec3 vel;
    vec3 acc;

    float radius;

    vec3 newAcc;
};

layout(std430, binding = 0) buffer Particles {
    Particle particles[];
};

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

out vec3 Normal;

uniform mat4 model;
uniform mat4 camMatrix;
uniform float scale; // Uniform for scaling

void main() {
    uint id = gl_InstanceID;
    vec3 particlePosition = particles[id].pos;

    // Translation matrix to move particle to the origin
    mat4 translateToOrigin = mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(-particlePosition, 1.0)
    );

    // Scaling matrix
    mat4 scaleMatrix = mat4(
        vec4(scale, 0.0, 0.0, 0.0),
        vec4(0.0, scale, 0.0, 0.0),
        vec4(0.0, 0.0, scale, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );

    // Translation matrix to move particle back to its original position
    mat4 translateBack = mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(particlePosition, 1.0)
    );

    // Combine transformations: Translate to origin -> Scale -> Translate back
    mat4 particleTransform = translateBack * scaleMatrix * translateToOrigin;

    // Apply the particle-specific transformation and the global model transformation
    mat4 finalModel = model * particleTransform;

    // Transform normals using the updated model matrix
    Normal = -normalize(mat3(transpose(inverse(finalModel))) * aNormal);

    // Compute final vertex position
    gl_Position = camMatrix * finalModel * vec4(aPos, 1.0);
}
