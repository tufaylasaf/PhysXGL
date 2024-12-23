#ifndef PARTICLE_CLASS_H
#define PARTICLE_CLASS_H

#include <random>

#include "model.h"

class Particle
{
public:
    glm::vec3 color;
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;

    float radius;

    Model model = Model("res/models/Shapes/icosphere.gltf", "s", true);

    static std::vector<Particle *> particles;

    Particle(float r, float cRadius)
    {
        color = glm::vec3(1.0f, 0.5f, 0.2f);
        pos = randomVec3(-cRadius, cRadius);
        vel = randomVec3(-1.0f, 1.0f);
        acc = glm::vec3(0.0f);

        r = radius;

        model.translation = glm::vec3(pos.y, pos.x, pos.z);

        particles.push_back(this);
    }

    void update(float dt, float g);

    void constraint(float r);

private:
    float randomFloat(float min, float max)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(min, max);
        return dis(gen);
    }

    glm::vec3 randomVec3(float min, float max)
    {
        return glm::vec3(randomFloat(min, max), randomFloat(min, max), randomFloat(min, max));
    }
};

#endif // !PARTICLE