cmake_minimum_required(VERSION 3.15)
project(LDtkViewer VERSION 0.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)
FetchContent_Declare(
        sogl
        GIT_REPOSITORY https://github.com/Madour/sogl
        GIT_TAG dev-0.2
)
FetchContent_MakeAvailable(sogl)

FetchContent_Declare(
        LDtkLoader
        GIT_REPOSITORY https://github.com/Madour/LDtkLoader
        GIT_TAG 1.1
)
FetchContent_MakeAvailable(LDtkLoader)

file(GLOB_RECURSE imgui_SRC imgui/*.cpp)
file(GLOB_RECURSE imgui_INC imgui/*.h)

add_library(ImGui STATIC ${imgui_SRC} ${imgui_INC})
set_target_properties(ImGui PROPERTIES DEBUG_POSTFIX -d)
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    target_link_libraries(ImGui PRIVATE glfw)
else()
    set_target_properties(ImGui PROPERTIES LINK_FLAGS "-s USE_GLFW=3 -s USE_WEBGL2=1 -s FULL_ES3=1")
endif()


file(GLOB_RECURSE SRC src/*.cpp)
file(GLOB_RECURSE INC src/*.hpp src/*.h)

add_executable(LDtkViewer ${SRC} ${INC})
target_include_directories(LDtkViewer PRIVATE src .)
target_link_libraries(LDtkViewer PRIVATE LDtkLoader sogl ImGui)
set_target_properties(LDtkViewer PROPERTIES DEBUG_POSTFIX -d)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    set(link_flags "-sUSE_GLFW=3 -sUSE_WEBGL2=1 -sFULL_ES3=1 -sALLOW_MEMORY_GROWTH=1 -sNO_DISABLE_EXCEPTION_CATCHING --shell-file ${CMAKE_SOURCE_DIR}/src/LDtkViewer.html")
    set_target_properties(LDtkViewer PROPERTIES LINK_FLAGS ${link_flags})
    set_target_properties(LDtkViewer PROPERTIES SUFFIX ".html")
endif()
