#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include <string>

#include "VAO.h"
#include "EBO.h"
#include "camera.h"
#include "texture.h"

struct Material
{
    glm::vec3 albedo = glm::vec3(1.0f, 1.0f, 1.0f);
    float roughness = 1.0f;
    float metallic = 1.0f;
    float ao = 1.0f;
};

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<Texture> textures;
    VAO VAO;

    Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices, std::vector<Texture> &textures);

    void Draw(
        Shader &shader,
        Camera &camera,
        glm::vec3 &translation,
        glm::quat &rotation,
        glm::vec3 &scale,
        Material &material,
        bool textured,
        glm::mat4 matrix = glm::mat4(1.0f));
};

#endif