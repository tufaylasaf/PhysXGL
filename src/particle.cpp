#include "particle.h"

void Particle::update(float dt, float g)
{
    acc.y -= g;

    vel += acc * dt;

    pos += vel * dt;

    acc = glm::vec3(0.0f);
}

void Particle::constraint(float r)
{
    float distance = glm::length(pos);

    if (distance > r)
    {
        pos = glm::normalize(pos) * r;

        glm::vec3 normal = glm::normalize(pos);
        vel -= glm::dot(vel, normal) * normal;
    }
}
