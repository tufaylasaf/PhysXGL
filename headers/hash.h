#ifndef HASH
#define HASH

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

class Hash
{
public:
    std::vector<int> firstAdjId;
    std::vector<int> adjIds;
    int querySize;
    std::vector<int> queryIds;

    Hash(float spacing, int maxNumObjects)
    {
        this->spacing = spacing;
        this->tableSize = 3 * maxNumObjects;

        this->cellCount.resize(this->tableSize + 1, 0);

        this->particleMap.resize(maxNumObjects, 0);
        this->queryIds.resize(maxNumObjects, 0);
        this->querySize = 0;

        this->maxNumObjects = maxNumObjects;

        this->firstAdjId.resize(maxNumObjects + 1);

        this->adjIds.resize(3 * maxNumObjects);
    }

    void create(std::vector<glm::vec3> pos)
    {
        std::fill(cellCount.begin(), cellCount.end(), 0);
        std::fill(particleMap.begin(), particleMap.end(), 0);

        for (int i = 0; i < pos.size(); i++)
        {
            const int h = hashPos(pos[i]);
            this->cellCount[h]++;
        }

        int start = 0;
        for (int i = 0; i < this->tableSize; i++)
        {
            start += this->cellCount[i];
            this->cellCount[i] = start;
        }
        this->cellCount[this->tableSize] = 0;

        for (int i = 0; i < pos.size(); i++)
        {
            const int h = hashPos(pos[i]);
            this->cellCount[h]--;
            this->particleMap[this->cellCount[h]] = i;
        }
    }

    void query(const glm::vec3 &pos, float maxDist)
    {
        const int x0 = intCoord(pos.x - maxDist);
        const int y0 = intCoord(pos.y - maxDist);
        const int z0 = intCoord(pos.z - maxDist);

        const int x1 = intCoord(pos.x + maxDist);
        const int y1 = intCoord(pos.y + maxDist);
        const int z1 = intCoord(pos.z + maxDist);

        this->querySize = 0;

        for (int xi = x0; xi <= x1; xi++)
        {
            for (int yi = y0; yi <= y1; yi++)
            {
                for (int zi = z0; zi <= z1; zi++)
                {
                    const int h = hashCoords(xi, yi, zi);

                    const int start = this->cellCount[h];
                    const int end = this->cellCount[h + 1];

                    for (int i = start; i < end; i++)
                    {
                        this->queryIds[this->querySize] = this->particleMap[i];
                        this->querySize++;
                    }
                }
            }
        }
    }

private:
    float spacing;
    int maxNumObjects;
    int tableSize;
    std::vector<int> cellCount;
    std::vector<int> particleMap;

    std::vector<int> getAdjacentParticles(int id) const
    {
        int start = firstAdjId[id];
        int end = firstAdjId[id + 1];
        return std::vector<int>(adjIds.begin() + start, adjIds.begin() + end);
    }

    int hashCoords(int xi, int yi, int zi) const
    {
        int h = (xi * 92837111) ^ (yi * 689287499) ^ (zi * 283923481); // Fantasy hash function
        return std::abs(h) % tableSize;
    }

    int intCoord(float coord) const
    {
        return static_cast<int>(std::floor(coord / spacing));
    }

    int hashPos(const glm::vec3 &pos) const
    {
        return hashCoords(
            intCoord(pos.x),
            intCoord(pos.y),
            intCoord(pos.z));
    }
};

#endif // !HASH