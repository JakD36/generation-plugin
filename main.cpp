//
// Created by Jack on 18/12/2021.
//

#if(_WIN32)
#include <windows.h>
#elif(APPLE)
#include <dlfcn.h>
#endif

#include <iostream>
#include "glm/glm.hpp"
#include <tuple>
#include <filesystem>
#include <chrono>



int main()
{
    auto cwd = std::filesystem::current_path();

    for (const auto& entry : std::filesystem::directory_iterator(cwd))
    {
        if(entry.is_regular_file())
        {
            std::string stemStr = entry.path().stem().string();
            bool startswith = stemStr.starts_with("generation-plugin");
            bool longerThanBase = stemStr.size() > strnlen_s("generation-plugin",100);
            bool isDll = entry.path().extension() == ".dll";
            if(startswith && longerThanBase && isDll)
            {
                std::filesystem::remove(entry);
            }
        }
    }

    glm::vec4 regionRect = glm::vec4(0,0,10,10);
    int size = 0;

    char* error;
#if(_WIN32)
    typedef glm::vec2* (__cdecl *POISSONDISCSAMPLING)(float, glm::vec4, int, int&);
    POISSONDISCSAMPLING PoissonDiscSampling;
#elif (APPLE)
    glm::vec2* (*PoissonDiscSampling)(float, glm::vec4, int, int&);
#endif

#if (_WIN32)
    std::stringstream ss;
    auto today = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(today);
    ss << tt;
    std::string newName = "generation-plugin_" + ss.str() + ".dll";
    std::filesystem::copy_file("generation-plugin.dll", newName);

    HMODULE handle = LoadLibrary(newName.c_str());
#elif (APPLE)
    void* handle = dlopen("libgeneration-plugin.dylib", RTLD_LOCAL);
#endif
    if(!handle)
    {
        std::cout << "Failed to load library" << std::endl;
        std::cout << error << std::endl;
        exit(2);
    }

#if (_WIN32)
    PoissonDiscSampling = (POISSONDISCSAMPLING)GetProcAddress(handle, "PoissonDiscSampling");
# elif (APPLE)
    *(void **) (&PoissonDiscSampling) = dlsym(handle, "PoissonDiscSampling");
#endif

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

#if (_WIN32)
            FreeLibrary(handle);
#elif (APPLE)
            dlclose(handle);
#endif


#if (_WIN32)
            std::stringstream ss;
            auto today = std::chrono::system_clock::now();
            std::time_t tt = std::chrono::system_clock::to_time_t(today);
            ss << tt;
            std::string newName = "generation-plugin_" + ss.str() + ".dll";
            std::filesystem::copy_file("generation-plugin.dll", newName);

            HMODULE handle = LoadLibrary(newName.c_str());
#elif (APPLE)
            void* handle = dlopen("libgeneration-plugin.dylib", RTLD_LOCAL);
#endif
            if(!handle)
            {
                std::cout << "Failed to load dylib" << std::endl;
                exit(2);
            }
#if (_WIN32)
            PoissonDiscSampling = (POISSONDISCSAMPLING)GetProcAddress(handle, "PoissonDiscSampling");
# elif (APPLE)
            *(void **) (&PoissonDiscSampling) = dlsym(handle, "PoissonDiscSampling");
#endif
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


#if (_WIN32)
    FreeLibrary(handle);
#elif (APPLE)
    dlclose(handle);
#endif
}