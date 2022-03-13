// Created by Modar Nasser on 12/03/2022.

#pragma once

#include "Camera2D.hpp"
#include "LDtkProject.hpp"

#include <sogl/sogl.hpp>

#include <map>
#include <vector>
#include <string>

class App {
public:
    App();
    bool loadLDtkFile(const char* path);
    void run();

private:
    void processEvent(sogl::Event& event);

    LDtkProject& getActiveProject();
    Camera2D& getActiveCamera();

    void initImGui();
    void renderImGui();

    sogl::Window m_window;
    sogl::Shader m_shader;

    std::string m_selected_project;
    std::map<std::string, LDtkProject> m_projects;
    std::map<std::string, Camera2D> m_cameras;

    LDtkProject m_dummy_project;

    static constexpr auto vert_shader = GLSL(330 core,
        uniform vec2 window_size = vec2(0.0, 0.0);
        uniform vec2 texture_size = vec2(0.0, 0.0);
        uniform vec3 transform = vec3(0.0, 0.0, 1.0);

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
            pos.xy *= 2*transform.z;

            tex.xy = i_tex.xy;
            if (texture_size.xy != vec2(0, 0))
                tex.xy /= texture_size.xy;
            else
                tex.xy = vec2(-1, -1);

            col = i_col;

            gl_Position = vec4(pos.x, -pos.y, 0, 1.0);
        }
    );
    static constexpr auto frag_shader = GLSL(330 core,
        uniform sampler2D texture0;
        uniform float opacity;

        in vec2 pos;
        in vec2 tex;
        in vec4 col;

        out vec4 gl_FragColor;

        void main() {
            vec4 tex_color = vec4(1.f, 1.f, 1.f, 1.f);
            if (tex.xy != vec2(-1, -1))
                tex_color = texture(texture0, tex);

            gl_FragColor = vec4(tex_color.rgb * col.rgb, tex_color.a * opacity * col.a);
        }
    );
};

