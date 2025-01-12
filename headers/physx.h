#ifndef PHYSX
#define PHYSX

#include "particle.h"
#include "hash.h"

class Physx
{
public:
    void update(float pr, float dt, float g, float r, int subSteps, Hash *hash);

    void resolveCollision(Particle *p1, Particle *p2, float pr, float dist);

private:
};

#endif // !PHYSX