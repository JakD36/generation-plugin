#include "library.h"

#include <vector>
#include <random>

//#include <iostream>

extern "C"
{
    glm::vec3 *Gen100Points()
    {
        return new glm::vec3[100];
    }

    void FreeArray(glm::vec3 *arr)
    {
        delete arr;
    }

    glm::vec2* PoissonDiscSampling(float radius, glm::vec4 regionRect, int samplesBeforeFail, int& size)
    {
        float cellSize = radius / glm::sqrt(2.f);
        glm::ivec2 gridSize = glm::ivec2((int) ceil(regionRect.z / cellSize), (int) ceil(regionRect.w / cellSize));

        int* grid = new int[gridSize.x * gridSize.y];
        std::fill(grid, grid + gridSize.x * gridSize.y, 0);


        std::vector<glm::vec2> points = std::vector<glm::vec2>();
        std::vector<glm::vec2> spawnPoints = std::vector<glm::vec2>
                {
                        glm::vec2(regionRect.x, regionRect.y) + glm::vec2(regionRect.z, regionRect.w) * 0.5f // Centre
                };

        std::random_device randDevice;
        std::default_random_engine generator(randDevice());

        while (spawnPoints.empty() == false) {
            std::uniform_int_distribution<int> distribution(0, spawnPoints.size()-1);
            int spawnIndex = distribution(generator);
            glm::vec2 centre = spawnPoints[spawnIndex];
            bool accepted = false;

            for (int i = 0; i < samplesBeforeFail; ++i) {

                std::uniform_real_distribution<float> angleDist(0, 2.f * 3.14f);
                float angle = angleDist(generator);
                glm::vec2 dir = glm::normalize(glm::vec2(glm::cos(angle), glm::sin(angle)));

//                std::cout << dir.x << " " << dir.y << std::endl;

                std::uniform_real_distribution<float> radialRand(radius, 2.f * radius);
                glm::vec2 candidate = centre + dir * radialRand(generator);

//                std::cout << candidate.x << " " << candidate.y << std::endl;

                glm::ivec2 cellIdx = glm::ivec2(
                        (int) ((candidate.x - regionRect.x) / cellSize),
                        (int) ((candidate.y - regionRect.y) / cellSize)
                );

                if (IsValid(candidate, regionRect, cellIdx, radius, points, grid, gridSize))
                {
                    points.push_back(candidate);
                    spawnPoints.push_back(candidate);
                    grid[cellIdx.x + cellIdx.y * gridSize.x] = points.size();
                    accepted = true;
                    break;
                }
            }

            if (accepted == false)
                spawnPoints.erase(begin(spawnPoints) + spawnIndex);

        }

        auto output = new glm::vec2[points.size()];
        for(int i = 0; i < points.size(); ++i)
        {
            output[i] = points[i]; // memcpy was causing issues
        }
        size = points.size();
        return output;
    }

    bool IsValid(glm::vec2 candidate, glm::vec4 region, glm::ivec2 cellIdx, float radius, const std::vector<glm::vec2> &points, int *grid, glm::ivec2 gridSize)
    {

        glm::vec2 rmin = glm::vec2(region.x, region.y);
        glm::vec2 rmax = glm::vec2(region.x + region.z, region.y + region.w);

        if (glm::all(glm::greaterThanEqual(candidate, rmin) && glm::lessThanEqual(candidate, rmax))) {
            int searchStartX = glm::max(0, cellIdx.x - 2);
            int searchEndX = glm::min(cellIdx.x + 2, gridSize.x - 1);
            int searchStartY = glm::max(0, cellIdx.y - 2);
            int searchEndY = glm::min(cellIdx.y + 2, gridSize.y - 1);

            for (int x = searchStartX; x <= searchEndX; ++x) {
                for (int y = searchStartY; y <= searchEndY; ++y) {
                    int pointIndex = grid[x + y * gridSize.x] - 1;
                    if (pointIndex != -1) {
                        if (glm::distance(candidate, points[pointIndex]) < radius)
                            return false;
                    }
                }
            }
            return true;
        }
        return false;
    }
}