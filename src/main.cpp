#include <array>
#include <iostream>

#include <sogl/sogl.hpp>

#include <LDtkLoader/World.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Camera2D.hpp"
#include "Layer.hpp"

int main() {
    auto vert_src = GLSL(330 core,
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
    auto frag_src = GLSL(330 core,
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

    sogl::Window window(1280, 720, "LDtkViewer");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(&window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui::GetStyle().WindowBorderSize = 0.f;

    sogl::Shader shader;
    shader.load(vert_src, frag_src);

    ldtk::World world;
    try {
        world.loadFromFile("../res/gridvania.ldtk");
    } catch(std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return 0;
    }

    std::vector<Layer> layers;
    for (const auto& level : world.allLevels()) {
        for (const auto& layer : level.allLayers()) {
            if (!layer.allTiles().empty())
                layers.emplace(layers.begin(), layer);
        }
    }

    Camera2D camera(window.getSize());

    bool camera_grabbed = false;
    glm::vec<2, int> grab_pos = {0, 0};
    auto world_bg = world.getBgColor();
    glm::vec<3, float> clear_color = {world_bg.r / 255.f, world_bg.g / 255.f, world_bg.b/255.f};

    bool show_demo_window = false;

    while (window.isOpen()) {
        auto mouse_pos = window.getMousePosition();

        while (auto event = window.nextEvent()) {
            if (auto resize = event->as<sogl::Event::Resize>()) {
                camera.setSize({resize->width, resize->height});
            }
            else if (auto key = event->as<sogl::Event::Key>()) {
                if (ImGui::GetIO().WantCaptureKeyboard)
                    continue;
                if (key->action == GLFW_PRESS) {
                    switch (key->key) {
                        case GLFW_KEY_ESCAPE:
                            window.close();
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (auto btn = event->as<sogl::Event::MouseButton>()) {
                if (ImGui::GetIO().WantCaptureMouse)
                    continue;
                if (btn->button == GLFW_MOUSE_BUTTON_LEFT) {
                    if (btn->action == GLFW_PRESS) {
                        camera_grabbed = true;
                        grab_pos = mouse_pos;
                    } else if (btn->action == GLFW_RELEASE) {
                        camera_grabbed = false;
                    }
                }
            }
            else if (auto move = event->as<sogl::Event::MouseMove>()) {
                if (camera_grabbed) {
                    auto dx = (grab_pos.x - move->x) / camera.getZoom();
                    auto dy = (grab_pos.y - move->y) / camera.getZoom();
                    grab_pos = {move->x, move->y};
                    camera.move(dx, dy);
                }
            }
            else if (auto scroll = event->as<sogl::Event::Scroll>()) {
                if (ImGui::GetIO().WantCaptureMouse)
                    continue;
                if (scroll->dy < 0) {
                    camera.zoom(0.9f);
                } else if (scroll->dy > 0) {
                    camera.zoom(1.1f);
                }
            }
        }

        window.clear(clear_color);

        shader.bind();
        shader.setUniform("window_size", glm::vec2(window.getSize()));
        shader.setUniform("transform", camera.getTransform());
        shader.setUniform("opacity", 1.f);

        for (auto& layer : layers) {
            shader.setUniform("texture_size", glm::vec2(layer.getTexture().getSize()));
            layer.render();
        }

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
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }
        {
            ImGui::SetNextWindowSize({200.f, (float)window.getSize().y});
            ImGui::SetNextWindowPos({0, 0});
            ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);
            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::End();
        }
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        window.display();
    }

    return 0;
}
