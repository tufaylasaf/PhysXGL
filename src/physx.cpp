#include "physx.h"

void Physx::update(float pr, float dt, float g, float r, int subSteps, Hash *hash)
{
    float subdt = dt / subSteps; // Divide time step into smaller intervals

    // for (int step = 0; step < subSteps; ++step)
    // {

    //     for (Particle *p1 : Particle::particles)
    //     {
    //         // Update particle properties for the current substep
    //         p1->radius = pr;
    //         p1->model.scale = glm::vec3(pr, pr, pr);
    //         p1->update(subdt, g);
    //         p1->constraint(r);

    //         for (Particle *p2 : Particle::particles)
    //         {
    //             if (p2 == p1)
    //                 continue;

    //             float dist = glm::distance(p1->pos, p2->pos);

    //             if (dist < pr * 2)
    //             {
    //                 resolveCollision(p1, p2, pr, dist);
    //             }
    //         }
    //     }
    // }

    for (Particle *p1 : Particle::particles)
    {
        // Set particle properties
        p1->radius = pr;
        p1->model.scale = glm::vec3(pr, pr, pr);

        hash->create(Particle::getPos());

        // Perform multiple substeps to increase accuracy
        for (int step = 0; step < subSteps; step++)
        {
            // Update the particle based on current time step
            p1->update(subdt, g);
            p1->constraint(r);

            // Query nearby particles using spatial hashing
            hash->query(p1->pos, 2.0f * pr);

            // Check for collisions with nearby particles
            for (int i = 0; i < hash->querySize; i++)
            {
                Particle *p2 = Particle::particles[hash->queryIds[i]];
                if (p1 == p2)
                    continue;

                float dist = glm::distance(p1->pos, p2->pos);

                // If particles are within collision range, resolve collision
                if (dist < pr * 2)
                {
                    resolveCollision(p1, p2, pr, dist);
                }
            }
        }
    }
}

void Physx::resolveCollision(Particle *p1, Particle *p2, float pr, float dist)
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