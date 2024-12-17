#include "light.h"

void Light::Draw(Shader &objectShader, Shader &lightShader, Camera &camera, bool onlySetShader)
{
    if (!onlySetShader)
        Model::Draw(objectShader, camera); // Draw the model as usual

    lightShader.Activate();
    glUniform1f(glGetUniformLocation(lightShader.ID, "pointLightCount"), pointLightCount);

    if (type == "Directional")
    {
        Light::Directional(lightShader);
    }
    else if (type == "Point")
    {
        Light::Point(lightShader, lightNum);
    }
    else
    {
        Light::Spot(lightShader);
    }
}

void Light::UI()
{
    if (ImGui::CollapsingHeader(name.c_str()))
    {
        // Position controls
        ImGui::Text("Transform");
        ImGui::DragFloat3("Position", &translation[0], 0.1f);

        ImGui::DragFloat3("Scale", &scale[0], 0.1f);

        ImGui::ColorEdit3("Color", &material.albedo[0]);

        if (type == "Directional")
        {
            ImGui::DragFloat3("Direction", &direction[0], 0.05f, -1.0f, 1.0f);
        }
        else if (type == "Point")
        {
            ImGui::Text("Attenuation");
            ImGui::SliderFloat("Constant", &constant, 0.0f, 1.0f);
            ImGui::SliderFloat("Linear", &linear, 0.0f, 2.0f);
            ImGui::SliderFloat("Quadratic", &quadratic, 0.0f, 2.0f);
        }
        else
        {
            ImGui::DragFloat3("Direction", &direction[0], 0.1f, -1.0f, 1.0f);
            ImGui::Text("Attenuation");
            ImGui::SliderFloat("Constant", &constant, 0.0f, 1.0f);
            ImGui::SliderFloat("Linear", &linear, 0.0f, 2.0f);
            ImGui::SliderFloat("Quadratic", &quadratic, 0.0f, 2.0f);
            ImGui::Text("Cutoff Angles");
            ImGui::SliderFloat("Cutoff", &cutoff, 0.0f, 90.0f);
            ImGui::SliderFloat("Outer Cutoff", &outerCutoff, cutoff, 90.0f);
        }
    }
}

void Light::Directional(Shader &shader)
{
    glUniform3f(glGetUniformLocation(shader.ID, "dLight.color"), material.albedo.x, material.albedo.y, material.albedo.z);
    glUniform3f(glGetUniformLocation(shader.ID, "dLight.direction"), direction.x, direction.y, direction.z);
}

void Light::Point(Shader &shader, int index)
{
    std::string baseName = "pLight[" + std::to_string(index) + "].";

    glUniform3f(glGetUniformLocation(shader.ID, (baseName + "color").c_str()), material.albedo.x, material.albedo.y, material.albedo.z);

    glUniform3f(glGetUniformLocation(shader.ID, (baseName + "position").c_str()), translation.x, translation.y, translation.z);
    glUniform1f(glGetUniformLocation(shader.ID, (baseName + "constant").c_str()), constant);
    glUniform1f(glGetUniformLocation(shader.ID, (baseName + "linear").c_str()), linear);
    glUniform1f(glGetUniformLocation(shader.ID, (baseName + "quadratic").c_str()), quadratic);
}

void Light::Spot(Shader &shader)
{
    glUniform3f(glGetUniformLocation(shader.ID, "sLight.ambient"), material.albedo.x, material.albedo.y, material.albedo.z);

    glUniform3f(glGetUniformLocation(shader.ID, "sLight.position"), translation.x, translation.y, translation.z);
    glUniform3f(glGetUniformLocation(shader.ID, "sLight.direction"), direction.x, direction.y, direction.z);
    glUniform1f(glGetUniformLocation(shader.ID, "sLight.constant"), constant);
    glUniform1f(glGetUniformLocation(shader.ID, "sLight.linear"), linear);
    glUniform1f(glGetUniformLocation(shader.ID, "sLight.quadratic"), quadratic);
    glUniform1f(glGetUniformLocation(shader.ID, "sLight.cutOff"), glm::cos(glm::radians(cutoff)));
    glUniform1f(glGetUniformLocation(shader.ID, "sLight.outerCutOff"), glm::cos(glm::radians(outerCutoff)));
}
