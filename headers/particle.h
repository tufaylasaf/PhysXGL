#ifndef PARTICLE_CLASS_H
#define PARTICLE_CLASS_H

#include "model.h"

class Particle
{
public:
    glm::vec3 color;
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 acc;

    void update(float dt, float g);

    void constraint(float r);
};

#endif // !PARTICLE