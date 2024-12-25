#include "particle.h"

void Particle::update(float dt, float g)
{

    newAcc = glm::vec3(0);

    applyForce(glm::vec3(0, -g, 0));

    glm::vec3 friction = -0.2f * vel;
    if (glm::length(vel) > 0.001f)
    {
        glm::vec3 frictionDir = glm::normalize(vel);
        glm::vec3 frictionForce = -frictionDir * 0.2f * glm::length(vel);
        applyForce(frictionForce);
    }

    glm::vec3 newPos = pos + vel * dt + acc * (dt * dt * 0.5f);

    glm::vec3 newVel = vel + (acc + newAcc) * (dt * 0.5f);

    pos = newPos;

    if (glm::length(newVel) < 0.001f)
    {
        vel = glm::vec3(0);
    }
    else
    {
        vel = newVel;
    }

    acc = newAcc;

    model.translation = glm::vec3(pos.y, pos.x, pos.z);
}

void Particle::applyForce(glm::vec3 f)
{
    newAcc += f;
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
