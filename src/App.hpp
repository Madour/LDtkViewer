// Created by Modar Nasser on 12/03/2022.

#pragma once

#include "AppImGui.hpp"
#include "LDtkProject/LDtkProjectObjects.hpp"
#include "LDtkProject/LDtkProject.hpp"

#include "imgui/imgui.h"

#include <LDtkLoader/World.hpp>
#include <sogl/sogl.hpp>

#include <functional>
#include <map>
#include <vector>
#include <string>

class App {
public:
    App();
    bool loadLDtkFile(const char* path);
    void unloadLDtkFile(const char* path);

    auto getWindow() -> sogl::Window&;

    auto allProjects() -> std::map<std::string, LDtkProject>&;

    bool projectOpened();

    void refreshActiveProject();
    LDtkProject& getActiveProject();
    void setActiveProject(LDtkProject& project);

    Camera2D& getCamera();

    void run();

private:
    void processEvent(sogl::Event& event);

    void renderActiveProject();

    sogl::Window m_window;
    sogl::Shader m_shader;

    AppImGui m_imgui;

    std::map<std::string, LDtkProject> m_projects;

    LDtkProject* m_selected_project = nullptr;

    static constexpr auto vert_shader = GLSL(330 core,
        precision highp float;
        uniform vec2 window_size;
        uniform vec2 texture_size;
        uniform vec3 transform;
        uniform vec2 offset;

        layout (location = 0) in vec2 i_pos;
        layout (location = 1) in vec2 i_tex;
        layout (location = 2) in vec4 i_col;

        out vec2 pos;
        out vec2 tex;
        out vec4 col;

        void main() {
            // normalize position
            pos.xy = i_pos.xy;
            if (window_size.xy != vec2(0, 0))
                pos.xy /= window_size.xy;

            // apply camera transform
            pos.xy += transform.xy;
            pos.xy *= 2.*transform.z;
            pos.xy += offset.xy / window_size.xy;

            tex.xy = i_tex.xy;
            if (tex.xy != vec2(-1., -1.) && texture_size.xy != vec2(0, 0))
                tex.xy /= texture_size.xy;
            else
                tex.xy = vec2(-1., -1.);

            col = i_col;

            gl_Position = vec4(pos.x, -pos.y, 0, 1.0);
        }
    );
    static constexpr auto frag_shader = GLSL(330 core,
        precision highp float;
        uniform sampler2D texture0;
        uniform vec4 color;

        in vec2 pos;
        in vec2 tex;
        in vec4 col;

        out vec4 fragColor;

        void main() {
            vec4 tex_color = vec4(1.f, 1.f, 1.f, 1.f);
            if (tex.xy != vec2(-1, -1))
                tex_color = texture(texture0, tex);

            fragColor = col * tex_color * color;
        }
    );
};

