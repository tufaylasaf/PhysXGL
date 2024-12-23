#include "physx.h"

void Physx::update(float pr, float dt, float g, float r)
{
    for (Particle *p1 : Particle::particles)
    {
        p1->radius = pr;
        p1->model.scale = glm::vec3(pr, pr, pr);
        p1->update(dt, 9.8f);
        p1->constraint(3.0f);

        for (Particle *p2 : Particle::particles)
        {
            if (p2 == p1)
                continue;

            float dist = glm::distance(p1->pos, p2->pos);

            if (dist <= pr * 2)
            {
                glm::vec3 n = (p2->pos - p1->pos) * ((pr * 2 - dist) / 2);

                p1->pos -= n;
                p2->pos += n;
            }
        }
    }
}