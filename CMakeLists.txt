cmake_minimum_required(VERSION 3.21)
project(Scatter)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory (extern EXCLUDE_FROM_ALL)

set(extern_libs
        glm
        )

#if(MSVC)
#    SET (CMAKE_CXX_FLAGS "-fdeclspec")
#endif()

add_library(Scatter SHARED
        src/SingleRadiusSampling.cpp
        src/MultiRadiiSampling.cpp
        )

target_link_libraries(Scatter
        ${extern_libs}
        )

add_executable(generation-test main.cpp ${files})

target_link_libraries(generation-test
        ${extern_libs}
        )