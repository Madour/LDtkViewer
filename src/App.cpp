// Created by Modar Nasser on 12/03/2022.

#include "App.hpp"
#include "TextureManager.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <LDtkLoader/World.hpp>

#include <filesystem>

App::App() : m_window(1280, 720, "LDtk World Viewer") {
    m_projects_data.insert({"", {}});
    m_shader.load(vert_shader, frag_shader);
    initImGui();
}

bool App::loadLDtkFile(const char* path) {
    m_projects.insert({path, {}});
    m_projects[path].load(path);

    m_projects_data.insert({path, {}});
    m_projects_data[path].camera.setSize(m_window.getSize());
    return true;
}

void App::run() {
    while (m_window.isOpen()) {
        while (auto event = m_window.nextEvent()) {
            processEvent(event.value());
        }

        if (m_selected_project.empty())
            m_window.clear({54.f/255.f, 60.f/255.f, 69.f/255.f});
        else
            m_window.clear(getActiveProject().bg_color);

        m_shader.bind();
        m_shader.setUniform("window_size", glm::vec2(m_window.getSize()));

        if (!m_selected_project.empty()) {
            const int active_depth = getActiveDepth();
            m_shader.setUniform("transform", getActiveCamera().getTransform());
            for (const auto& world : getActiveProject().worlds) {
                for (const auto& [depth, levels] : world.levels) {
                    if (depth > active_depth)
                        continue;
                    float opacity = 0.5f - std::abs(active_depth - depth)/4.f;
                    m_shader.setUniform("opacity", depth == active_depth ? 1.f : opacity);
                    for (const auto& level : levels) {
                        for (const auto& layer : level.layers)
                            layer.render(m_shader);
                    }
                }
            }
        }

        renderImGui();

        m_window.display();
    }
}

void App::processEvent(sogl::Event& event) {
    static bool camera_grabbed = false;
    static glm::vec<2, int> grab_pos;

    if (auto resize = event.as<sogl::Event::Resize>()) {
        for (auto& [_, data] : m_projects_data)
            data.camera.setSize({resize->width, resize->height});
    }
    else if (auto drop = event.as<sogl::Event::Drop>()) {
        for (auto& file : drop->files)
            loadLDtkFile(file.c_str());
    }
    else if (auto key = event.as<sogl::Event::Key>()) {
        if (!ImGui::GetIO().WantCaptureKeyboard) {
            if (key->action == GLFW_PRESS) {
                switch (key->key) {
                    case GLFW_KEY_ESCAPE:
                        m_window.close();
                        break;
                    default:
                        break;
                }
            }
        }
    }
    else if (auto btn = event.as<sogl::Event::MouseButton>()) {
        if (!ImGui::GetIO().WantCaptureMouse) {
            if (btn->button == GLFW_MOUSE_BUTTON_LEFT) {
                if (btn->action == GLFW_PRESS) {
                    camera_grabbed = true;
                    grab_pos = m_window.getMousePosition();
                } else if (btn->action == GLFW_RELEASE) {
                    camera_grabbed = false;
                }
            } else if (btn->button == GLFW_MOUSE_BUTTON_RIGHT) {
                if (btn-> action == GLFW_PRESS) {
                    auto& active_project_levels = getActiveProject().worlds[0].levels;
                    auto depth_offset = active_project_levels.begin()->first;
                    auto depth = getActiveDepth()+1 - depth_offset;
                    m_projects_data[m_selected_project].depth = (depth % active_project_levels.size()) + depth_offset;
                }
            }
        }
    }
    else if (auto move = event.as<sogl::Event::MouseMove>()) {
        if (camera_grabbed) {
            auto& camera = getActiveCamera();
            auto dx = (grab_pos.x - move->x) / camera.getZoom();
            auto dy = (grab_pos.y - move->y) / camera.getZoom();
            grab_pos = {move->x, move->y};
            camera.move(dx, dy);
        }
    }
    else if (auto scroll = event.as<sogl::Event::Scroll>()) {
        if (!ImGui::GetIO().WantCaptureMouse) {
            auto& camera = getActiveCamera();
            if (scroll->dy < 0) {
                camera.zoom(0.9f);
            } else if (scroll->dy > 0) {
                camera.zoom(1.1f);
            }
        }
    }
}

LDtkProject& App::getActiveProject() {
    if (m_selected_project.empty())
        return m_dummy_project;
    return m_projects.at(m_selected_project);
}

Camera2D& App::getActiveCamera() {
    return m_projects_data.at(m_selected_project).camera;
}

int App::getActiveDepth() {
    return m_projects_data.at(m_selected_project).depth;
}

void App::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(&m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui::GetStyle().WindowBorderSize = 0.f;
}

void App::renderImGui() {
    static bool show_demo_window = false;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);
    {
        static float f = 0.0f;
        static float color[3];
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", color);                // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }
    {
        static std::map<std::string, bool> worlds_tabs;
        ImGui::SetNextWindowSize({(float)m_window.getSize().x-200.f, 20.});
        ImGui::SetNextWindowPos({200.f, 0});
        ImGui::Begin("Full", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration
                                    | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
        ImGui::BeginTabBar("WorldsSelector");
        worlds_tabs.clear();
        for (auto& [name, _] : m_projects) {
            worlds_tabs[name] = true;
            auto filename = std::filesystem::path(name).filename().string();
            if (ImGui::BeginTabItem(filename.c_str(), &worlds_tabs[name])) {
                m_selected_project = name;
                ImGui::EndTabItem();
            }
        }
        for (auto& [name, open] : worlds_tabs) {
            if (!open) {
                m_projects.erase(name);
                m_projects_data.erase(name);
                m_selected_project.clear();
            }
        }

        ImGui::EndTabBar();
        ImGui::End();
    }
    {
        ImGui::SetNextWindowSize({200.f, (float)m_window.getSize().y});
        ImGui::SetNextWindowPos({0, 0});
        ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);
        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::End();
    }
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}