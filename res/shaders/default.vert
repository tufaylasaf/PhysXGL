#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace; // Position in light space

uniform mat4 camMatrix;
uniform mat4 model;
uniform mat4 lightProjection; // Light's view-projection matrix

void main()
{
    // Compute the fragment position in world space
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Compute normal with respect to model matrix transformations
    Normal = -normalize(mat3(transpose(inverse(model))) * aNormal);

    // Adjust texture coordinates
    TexCoords = mat2(0.0, -1.0, 1.0, 0.0) * aTex;

    // Compute the fragment position in light space
    FragPosLightSpace = lightProjection * vec4(FragPos, 1.0);

    // Calculate the final vertex position in camera space
    gl_Position = camMatrix * vec4(FragPos, 1.0);
}
