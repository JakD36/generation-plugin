#ifndef SCATTER_POISSONSCATTERING_H
#define SCATTER_POISSONSCATTERING_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#ifdef Scatter_EXPORTS
#define SCATTER_API __declspec(dllexport)
#else
#define SCATTER_API __declspec(dllimport)
#endif
#else
#define SCATTER_API
#endif

#include "glm/glm.hpp"
#include <vector>

struct PointData2
{
    glm::vec2 m_pos;
    uint16_t m_index;
};

struct PointData3
{
    glm::vec3 m_pos;
    uint16_t m_index;
    glm::vec3 m_normal;
};

extern "C"
{
    SCATTER_API glm::vec2* PoissonDiscSampling(float radius, glm::vec4 regionRect, int samplesBeforeFail, uint32_t& size);
    SCATTER_API PointData2* PoissonDiscSamplingMultiRadii(float* radii, float* distributions, int inputSize, glm::vec4 regionRect, int samplesBeforeFail, int32_t& outputSize);
    SCATTER_API PointData3* PoissonDiscSamplingMultiRadiiHeight(float* radii, float* distributions, glm::vec2* acceptedSlopes, int inputSize, glm::vec4 regionRect, int16_t* heightmap, glm::ivec2 heightmapResolution, glm::vec3 heightmapScale, int samplesBeforeFail, int32_t& outputSize);
}

bool IsValid(glm::vec2 candidate, glm::vec4 region, glm::ivec2 cellIdx, float radius, const std::vector<glm::vec2> &points, int *grid, glm::ivec2 gridSize);
bool IsValidMultiRadii(glm::vec2 candidate, glm::vec4 region, glm::ivec2 cellIdx, float radius, float* radii, const std::vector<PointData2> &points, std::vector<int>* grid, glm::ivec2 gridSize);

#endif
