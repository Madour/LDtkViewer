// Created by Modar Nasser on 12/03/2022.

#include "App.hpp"
#include "TextureManager.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <LDtkLoader/World.hpp>


App::App() : m_window(1280, 720, "LDtk World Viewer") {
    m_shader.load(vert_shader, frag_shader);
    m_camera.setSize(m_window.getSize());
    m_clear_color = {54.f/255.f, 60.f/255.f, 69.f/255.f};

    initImGui();
}

void App::run() {
    while (m_window.isOpen()) {
        while (auto event = m_window.nextEvent()) {
            processEvent(event.value());
        }

        m_window.clear(m_clear_color);

        m_shader.bind();
        m_shader.setUniform("window_size", glm::vec2(m_window.getSize()));
        m_shader.setUniform("transform", m_camera.getTransform());
        m_shader.setUniform("opacity", 1.f);

        for (auto& layer : m_layers) {
            m_shader.setUniform("texture_size", glm::vec2(layer.getTexture().getSize()));
            layer.render();
        }

        renderImGui();

        m_window.display();
    }
}

void App::processEvent(sogl::Event& event) {
    static bool camera_grabbed = false;
    static glm::vec<2, int> grab_pos;

    if (auto resize = event.as<sogl::Event::Resize>()) {
        m_camera.setSize({resize->width, resize->height});
    }
    else if (auto drop = event.as<sogl::Event::Drop>()) {
        loadLDtkFile(drop->files[0].c_str());
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
            }
        }
    }
    else if (auto move = event.as<sogl::Event::MouseMove>()) {
        if (camera_grabbed) {
            auto dx = (grab_pos.x - move->x) / m_camera.getZoom();
            auto dy = (grab_pos.y - move->y) / m_camera.getZoom();
            grab_pos = {move->x, move->y};
            m_camera.move(dx, dy);
        }
    }
    else if (auto scroll = event.as<sogl::Event::Scroll>()) {
        if (!ImGui::GetIO().WantCaptureMouse) {
            if (scroll->dy < 0) {
                m_camera.zoom(0.9f);
            } else if (scroll->dy > 0) {
                m_camera.zoom(1.1f);
            }
        }
    }
}

bool App::loadLDtkFile(const char* path) {
    ldtk::World world;
    try {
        world.loadFromFile(path);
    } catch(std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }

    auto& bg = world.getBgColor();
    m_clear_color = {bg.r/255.f, bg.g/255.f, bg.b/255.f};

    TextureManager::clear();
    m_layers.clear();
    for (const auto& level : world.allLevels()) {
        for (const auto& layer : level.allLayers()) {
            if (!layer.allTiles().empty())
                m_layers.emplace(m_layers.begin(), layer);
        }
    }
    return true;
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
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&m_clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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