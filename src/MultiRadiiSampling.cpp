#include "PoissonScattering.h"

#include <vector>
#include <random>

using namespace std;

PointData2* PoissonDiscSamplingMultiRadii(float* radii, float* distributions, int inputSize, glm::vec4 regionRect, int samplesBeforeFail, int32_t& outputSize)
{
    float maxRadius = FLT_MIN;
    for(int i = 0; i < inputSize; ++i)
    {
        maxRadius = fmax(radii[i], maxRadius);
    }

    float cellSize = maxRadius / glm::sqrt(2.f);
    glm::ivec2 gridSize = glm::ivec2((int) ceil(regionRect.z / cellSize), (int) ceil(regionRect.w / cellSize));

    vector<int>* grid = new vector<int>[gridSize.x * gridSize.y];
    for(int i = 0; i < gridSize.x * gridSize.y; ++i)
    {
        grid[i] = vector<int>();
    }

    random_device randDevice;
    default_random_engine generator(randDevice());
    uniform_real_distribution<float> radiiDistribution(0.f, 1.f);

    uint16_t radiiIndex = 0;
    float rand = radiiDistribution(generator);
    for(uint16_t j = 0; j < inputSize; ++j)
    {
        if (rand < distributions[j])
        {
            radiiIndex = j;
            break;
        }
    }

    vector<PointData2> points = vector<PointData2>();
    vector<PointData2> spawnPoints = vector<PointData2>
    {
        PointData2
        {
            glm::vec2(regionRect.x, regionRect.y) + glm::vec2(regionRect.z, regionRect.w) * 0.5f, // Centre
            radiiIndex
        }
    };


    while (spawnPoints.empty() == false)
    {
        // Pick the next point as the centre
        uniform_int_distribution<int> distribution(0, spawnPoints.size()-1);
        int spawnIndex = distribution(generator);
        PointData2& centre = spawnPoints[spawnIndex];

        // Pick the next radius
        rand = radiiDistribution(generator);
        for(uint16_t j = 0; j < inputSize; ++j)
        {
            if (rand < distributions[j])
            {
                radiiIndex = j;
                break;
            }
        }

        bool accepted = false;

        for (int i = 0; i < samplesBeforeFail; ++i)
        {
            uniform_real_distribution<float> angleDist(0, 2.f * 3.14f);
            float angle = angleDist(generator);
            glm::vec2 dir = glm::normalize(glm::vec2(glm::cos(angle), glm::sin(angle)));

            uniform_real_distribution<float> radialRand(radii[centre.m_index],  radii[centre.m_index] + radii[radiiIndex]);
            glm::vec2 candidate = centre.m_pos + dir * radialRand(generator);

            glm::ivec2 cellIdx = glm::ivec2(
                (int) ((candidate.x - regionRect.x) / cellSize),
                (int) ((candidate.y - regionRect.y) / cellSize)
            );

            if (IsValidMultiRadii(candidate, regionRect, cellIdx, radii[radiiIndex], radii, points, grid, gridSize))
            {
                auto newPoint = PointData2
                        {
                            candidate,
                            radiiIndex
                        };

                grid[cellIdx.x + cellIdx.y * gridSize.x].push_back(points.size());
                points.push_back(newPoint);
                spawnPoints.push_back(newPoint);
                accepted = true;
                break;
            }
        }

        if (accepted == false)
        {
            spawnPoints[spawnIndex] = spawnPoints.back();
            spawnPoints.pop_back();
        }
    }

    outputSize = points.size();
    PointData2* output = new PointData2[outputSize];
    for(int i = 0; i < outputSize; ++i)
    {
        output[i] = points[i];
    }
    return output;
}

bool IsValidMultiRadii(glm::vec2 candidate, glm::vec4 region, glm::ivec2 cellIdx, float radius, float* radii, const vector<PointData2>& points, vector<int>* grid, glm::ivec2 gridSize)
{

    glm::vec2 rmin = glm::vec2(region.x, region.y);
    glm::vec2 rmax = glm::vec2(region.x + region.z, region.y + region.w);

    if (glm::all(glm::greaterThanEqual(candidate, rmin) && glm::lessThanEqual(candidate, rmax)))
    {
        int searchStartX = glm::max(0, cellIdx.x - 2);
        int searchEndX = glm::min(cellIdx.x + 2, gridSize.x - 1);
        int searchStartY = glm::max(0, cellIdx.y - 2);
        int searchEndY = glm::min(cellIdx.y + 2, gridSize.y - 1);

        for (int y = searchStartY; y <= searchEndY; ++y)
        {
            for (int x = searchStartX; x <= searchEndX; ++x)
            {
                for(int i = 0, size = grid[x + y * gridSize.x].size(); i < size; ++i)
                {
                    int pointIndex = grid[x + y * gridSize.x][i];
                    float maxRadius = fmax(radius, radii[points[pointIndex].m_index]);
                    glm::vec2 vec = candidate - points[pointIndex].m_pos;
                    if (glm::dot(vec, vec) < maxRadius * maxRadius)
                        return false;
                }
            }
        }
        return true;
    }
    return false;
}
