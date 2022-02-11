#ifndef UNITY_DLL_LIBRARY_H
#define UNITY_DLL_LIBRARY_H

#if _MSC_VER // this is defined when compiling with Visual Studio
    #define EXPORT __declspec(dllexport) // Visual Studio needs annotating exported functions with this
#else
    #define EXPORT // XCode does not need annotating exported functions, so define is empty
#endif

#include "glm/glm.hpp"
#include <tuple>
#include <vector>

extern "C"
{
    EXPORT glm::vec3 *Gen100Points();
    EXPORT void FreeArray(glm::vec3 *arr);

    EXPORT glm::vec2* PoissonDiscSampling(float radius, glm::vec4 regionRect, int samplesBeforeFail, int& size);
    bool IsValid(glm::vec2 candidate, glm::vec4 region, glm::ivec2 cellIdx, float radius, const std::vector<glm::vec2> &points, int *grid, glm::ivec2 gridSize);
}
#endif //UNITY_DLL_LIBRARY_H
