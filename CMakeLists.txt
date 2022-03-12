cmake_minimum_required(VERSION 3.12)
project(LDtkViewer)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
        sogl
        GIT_REPOSITORY https://github.com/Madour/sogl
        GIT_TAG master
)
FetchContent_MakeAvailable(sogl)

find_package(LDtkLoader 0.10)

if(MSVC)
    add_compile_options(
            $<$<CONFIG:>:/MD> #---------|
            $<$<CONFIG:Debug>:/MDd> #---|-- Statically link the runtime libraries
            $<$<CONFIG:Release>:/MD> #--|
    )
endif()

file(GLOB_RECURSE imgui_SRC imgui/*.cpp)
file(GLOB_RECURSE imgui_INC imgui/*.h)

add_library(ImGui STATIC ${imgui_SRC} ${imgui_INC})
set_target_properties(ImGui PROPERTIES DEBUG_POSTFIX -d)
target_link_libraries(ImGui PRIVATE glfw)

file(GLOB_RECURSE SRC src/*.cpp)
file(GLOB_RECURSE INC src/*.hpp)

add_executable(LDtkViewer ${SRC} ${INC})
target_include_directories(LDtkViewer PRIVATE src .)
target_link_libraries(LDtkViewer PRIVATE LDtkLoader sogl ImGui)
set_target_properties(LDtkViewer PROPERTIES DEBUG_POSTFIX -d)
