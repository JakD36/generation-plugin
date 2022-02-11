//
// Created by Jack on 18/12/2021.
//

// #include <stdlib.h>
#include <dlfcn.h>
#include <iostream>
#include "glm/glm.hpp"
#include <tuple>
#include <vector>

int main()
{
    glm::vec4 regionRect = glm::vec4(0,0,10,10);
    int size = 0;

    void* handle;
    char* error;
    glm::vec2* (*PoissonDiscSampling)(float, glm::vec4, int, int&);


    handle = dlopen("libgeneration-plugin.dylib", RTLD_LOCAL);
    if(!handle)
    {
        std::cout << "Failed to load dylib" << std::endl;
        exit(2);
    }

    *(void **) (&PoissonDiscSampling) = dlsym(handle, "PoissonDiscSampling");
    
    while(true)
    {
        std::string s;
        std::cin >> s;
        if(s == "close")
        {
            break;
        }
        else if (s == "reload")
        {
            std::cout << "Attempting to Reload dylib" << std::endl;
            dlclose(handle);

            handle = dlopen("libgeneration-plugin.dylib", RTLD_LOCAL);
            if(!handle)
            {
                std::cout << "Failed to load dylib" << std::endl;
                exit(2);
            }
            *(void **) (&PoissonDiscSampling) = dlsym(handle, "PoissonDiscSampling");
        }
        else if(s == "run")
        {
            glm::vec2* arr = PoissonDiscSampling(1.f, regionRect, 30, size);
            for (int i = 0; i < size; ++i)
            {
                std::cout << arr[i].x << " " << arr[i].y << std::endl;
            }
            std::cout << size << std::endl;
            delete arr;
        }
    }
    

    dlclose(handle);
}