IF(MSVC)
    SET(CMAKE_CXX_FLAGS "/EHsc")
ENDIF(MSVC)

set(BUILD_SHARED_LIBS OFF)

add_subdirectory(glm ${CMAKE_BINARY_DIR}/extern/glm)


