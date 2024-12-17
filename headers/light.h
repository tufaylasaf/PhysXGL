#ifndef LIGHT_CLASS_H
#define LIGHT_CLASS_H

#include "model.h"
class Light : public Model
{
public:
    static int pointLightCount; // Add this to track the number of point lights
    int lightNum = 0;
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float cutoff = 20.0f;
    float outerCutoff = 40.0f;

    std::string type;

    static std::vector<Light *> lights;

    Light(const char *file, std::string n, std::string t) : type(t), Model(file, n, false)
    {
        lights.push_back(this);
        if (t == "Point")
        {
            lightNum = pointLightCount;
            pointLightCount++;
        } // Increase the point light counter if it's a point light
    }

    void Draw(Shader &objectShader, Shader &lightShader, Camera &camera, bool onlySetShader);

    void UI();

private:
    void Directional(Shader &shader);
    void Point(Shader &shader, int index); // Modify this to take the light index
    void Spot(Shader &shader);
};

// Initialize static variable

#endif
