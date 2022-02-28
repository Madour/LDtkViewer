cmake_minimum_required(VERSION 3.12)
project(LDtkViewer)

set(CMAKE_CXX_STANDARD 17)

message(STATUS "Downloading dependencies ...")

include(FetchContent)
FetchContent_Declare(
        glew
        GIT_REPOSITORY https://github.com/Perlmint/glew-cmake/
        GIT_TAG glew-cmake-2.2.0
)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.3.6
)
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG 0.9.9.8
)

if(NOT glew_POPULATED)
    FetchContent_Populate(glew)
    set(glew-cmake_BUILD_SHARED OFF CACHE INTERNAL "")
    set(glew-ONLY_LIBS ON CACHE INTERNAL "")
    add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR})
endif()

if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "")
    set(GLFW_INSTALL OFF CACHE INTERNAL "")
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

if(NOT glm_POPULATED)
    FetchContent_Populate(glm)
    set(GLM_CONFIG_XYZW_ONLY ON CACHE INTERNAL "")
    add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})
endif()

file(GLOB_RECURSE SRC src/*.cpp )
file(GLOB_RECURSE INC src/*.hpp )

add_executable(LDtkViewer ${SRC} ${INC})
target_link_libraries(LDtkViewer PRIVATE libglew_static glfw glm)
set_target_properties(LDtkViewer PROPERTIES DEBUG_POSTFIX -d)
