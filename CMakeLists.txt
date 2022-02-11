cmake_minimum_required(VERSION 3.21)
project(unity_dll)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory (extern EXCLUDE_FROM_ALL)

set(extern_libs
        glm
        )

#if(MSVC)
#    SET (CMAKE_CXX_FLAGS "-fdeclspec")
#endif()

#add_library(unity_dll SHARED library.cpp library.h)
file(GLOB files src/*.cpp src/*.h src/*.hpp)

add_library(generation-plugin SHARED ${files})

target_link_libraries(generation-plugin
        ${extern_libs}
        )


add_executable(generation-test main.cpp ${files})

target_link_libraries(generation-test
        ${extern_libs}
        )