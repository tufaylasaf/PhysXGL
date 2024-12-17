#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<GLuint> &indices, std::vector<Texture> &textures)
{
    Mesh::vertices = vertices;
    Mesh::indices = indices;
    Mesh::textures = textures;

    VAO.Bind();
    VBO VBO(vertices);
    EBO EBO(indices);
    VAO.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void *)0);
    VAO.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void *)(3 * sizeof(float)));
    VAO.LinkAttrib(VBO, 2, 2, GL_FLOAT, sizeof(Vertex), (void *)(6 * sizeof(float)));
    VAO.Unbind();
    VBO.Unbind();
    EBO.Unbind();
}

void Mesh::Draw(
    Shader &shader,
    Camera &camera,
    glm::vec3 &translation,
    glm::quat &rotation,
    glm::vec3 &scale,
    Material &material,
    bool textured,
    glm::mat4 matrix) // Pass by reference to allow modification
{
    shader.Activate();
    VAO.Bind();
    glUniform1i(glGetUniformLocation(shader.ID, "textured"), textured);

    unsigned int numDiffuse = 0;
    unsigned int numSpecular = 0;

    // Bind textures
    for (unsigned int i = 0; i < textures.size(); i++)
    {
        std::string num;
        std::string type = textures[i].type;
        if (type == "diffuse")
        {
            num = std::to_string(numDiffuse++);
        }
        else if (type == "specular")
        {
            num = std::to_string(numSpecular++);
        }
        else
        {
            num = "Map";
        }
        textures[i].texUnit(shader, (type + num).c_str(), i + 3);
        textures[i].Bind();
    }

    // Set camera position and view matrix
    glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
    camera.Matrix(shader, "camMatrix");

    // Create transformation matrices
    matrix = glm::translate(matrix, translation);
    matrix *= glm::mat4_cast(rotation);
    matrix = glm::scale(matrix, scale);

    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(matrix));
    glUniform3f(glGetUniformLocation(shader.ID, "material.albedo"), material.albedo.x, material.albedo.y, material.albedo.z);
    glUniform1f(glGetUniformLocation(shader.ID, "material.metallic"), material.metallic);
    glUniform1f(glGetUniformLocation(shader.ID, "material.roughness"), material.roughness);
    glUniform1f(glGetUniformLocation(shader.ID, "material.ao"), material.ao);

    // Draw the mesh
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
}
