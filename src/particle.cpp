#include "particle.h"

void Particle::update(float dt, float g)
{

    acc.y -= g;

    glm::vec3 friction = -0.2f * vel;
    acc += friction;

    vel += acc * dt;
    pos += vel * dt;

    model.translation = glm::vec3(pos.y, pos.x, pos.z);

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
