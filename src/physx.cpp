#include "physx.h"

void Physx::update(float pr, float dt, float g, float r)
{
    for (Particle *p1 : Particle::particles)
    {
        p1->radius = pr;
        p1->model.scale = glm::vec3(pr, pr, pr);
        p1->update(dt, g);
        p1->constraint(r);

        for (Particle *p2 : Particle::particles)
        {
            if (p2 == p1)
                continue;

            float dist = glm::distance(p1->pos, p2->pos);
            if (dist < pr * 2)
            {
                float overlap = pr * 2 - dist;
                glm::vec3 collisionNormal = glm::normalize(p2->pos - p1->pos);
                glm::vec3 correction = collisionNormal * (overlap / 2.0f);
                p1->pos -= correction;
                p2->pos += correction;

                glm::vec3 relativeVel = p1->vel - p2->vel;
                float impulseMagnitude = glm::dot(relativeVel, collisionNormal);

                float restitution = 0.8f;
                glm::vec3 impulse = collisionNormal * impulseMagnitude * restitution;

                p1->vel -= impulse * 0.5f;
                p2->vel += impulse * 0.5f;
            }
        }
    }
}
