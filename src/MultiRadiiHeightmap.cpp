#include "PoissonScattering.h"

#include <vector>
#include <random>

using namespace std;

float SampleHeight(const glm::vec2& pos, const glm::vec4& terrainRect, int16_t* heightmap, const glm::ivec2& heightmapResolution, const glm::vec3& heightmapScale);
glm::vec3 SampleNormal(const glm::vec2& pos, const glm::vec4& terrainRect, int16_t* heightmap, const glm::ivec2& heightmapResolution, const glm::vec3& heightmapScale);

PointData3* PoissonDiscSamplingMultiRadiiHeight(float* radii, float* distributions, glm::vec2* acceptedSlopes, int inputSize, glm::vec4 regionRect, int16_t* heightmap, glm::ivec2 heightmapResolution, glm::vec3 heightmapScale, int samplesBeforeFail, int32_t& outputSize)
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

    // Pick the first radii
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


    vector<glm::vec3> normals = vector<glm::vec3>();
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

            glm::ivec2 cellIdx = glm::ivec2((int) ((candidate.x - regionRect.x) / cellSize),(int) ((candidate.y - regionRect.y) / cellSize));

            glm::vec3 normal = SampleNormal(candidate, regionRect, heightmap, heightmapResolution, heightmapScale);
            float gradient = glm::clamp(glm::dot(glm::vec3(0.f,1.f,0.f), normal),0.f,1.f);
            if((acceptedSlopes[radiiIndex].x <= gradient && gradient <= acceptedSlopes[radiiIndex].y) == false)
                continue;

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
                normals.push_back(normal);
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
    PointData3* output = new PointData3[outputSize];
    for(int i = 0; i < outputSize; ++i)
    {
        float height = SampleHeight(points[i].m_pos, regionRect, heightmap, heightmapResolution, heightmapScale);
        output[i] = PointData3
                {
            glm::vec3(points[i].m_pos.x, height,points[i].m_pos.y),
            points[i].m_index,
            normals[i]
                };
    }
    return output;
}


/// Gets the height
///
float SampleHeight(const glm::vec2& pos, const glm::vec4& terrainRect, int16_t* heightmap, const glm::ivec2& heightmapResolution, const glm::vec3& heightmapScale)
{
    glm::vec2 localPos = glm::max(pos - glm::vec2(terrainRect.x, terrainRect.y), glm::vec2(0.f,0.f));
    float xPos = glm::min((localPos.x / terrainRect.z) * (float)heightmapResolution.x, (float)heightmapResolution.x);
    float zPos = glm::min((localPos.y / terrainRect.w) * (float)heightmapResolution.y, (float)heightmapResolution.y);

    int xInt = glm::floor(xPos);
    int zInt = glm::floor(zPos);
    float xFrac = glm::fract(xPos);
    float zFrac = glm::fract(zPos);
    int xCeil = glm::ceil(xPos);
    int zCeil = glm::ceil(zPos);

    float p0 = static_cast<float>(heightmap[xInt + zInt * heightmapResolution.x]) / SHRT_MAX;
    float p1 = static_cast<float>(heightmap[xCeil + zInt * heightmapResolution.x]) / SHRT_MAX;
    float p2 = static_cast<float>(heightmap[xInt + zCeil * heightmapResolution.x]) / SHRT_MAX;
    float p3 = static_cast<float>(heightmap[xCeil + zCeil * heightmapResolution.x]) / SHRT_MAX;

    float v0 = lerp(p0,p1,xFrac);
    float v1 = lerp(p2,p3, xFrac);
    float v2 = lerp(v0,v1, zFrac);

    return v2 * heightmapScale.y;
}


// todo there seems to be an issue with these up, left, right, down go out of bounds of the heightmap index
glm::vec3 SampleNormal(const glm::vec2& pos, const glm::vec4& terrainRect, int16_t* heightmap, const glm::ivec2& heightmapResolution, const glm::vec3& heightmapScale)
{
    const float width = heightmapScale.x;
    const float height = heightmapScale.z;

    glm::vec2 right = glm::vec2(pos.x + width, pos.y);
    glm::vec2 left = glm::vec2(pos.x - width, pos.y);
    glm::vec2 up = glm::vec2(pos.x, pos.y + height);
    glm::vec2 down = glm::vec2(pos.x, pos.y - height);

    float hRight = SampleHeight(right, terrainRect, heightmap, heightmapResolution, heightmapScale);
    float hLeft = SampleHeight(left, terrainRect, heightmap, heightmapResolution, heightmapScale);
    float hUp = SampleHeight(up, terrainRect, heightmap, heightmapResolution, heightmapScale);
    float hDown = SampleHeight(down, terrainRect, heightmap, heightmapResolution, heightmapScale);

    glm::vec3 normal = glm::vec3(
            (hLeft - hRight) / (heightmapScale.x * 2.f),
            1.f,
            (hDown - hUp) / (heightmapScale.z * 2.f)
            );
    normal = glm::normalize(normal);
    return normal;
}