//
// Created by Jack on 18/12/2021.
//

#include "src/library.h"
#include <iostream>

int main()
{

    glm::vec4 regionRect = glm::vec4(0,0,10,10);
    int size = 0;

    glm::vec2* arr = PoissonDiscSampling(1.f, regionRect, 30, size);

    for (int i = 0; i < size; ++i)
    {
        std::cout << arr[i].x << " " << arr[i].y << std::endl;
    }

    std::cout << size << std::endl;
}