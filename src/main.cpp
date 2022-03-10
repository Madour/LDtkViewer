#include <array>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <LDtkLoader/World.hpp>

#include "sogl/Shader.hpp"
#include "sogl/Texture.hpp"
#include "sogl/VertexArray.hpp"
#include "sogl/Window.hpp"

#include "Camera2D.hpp"
#include "Layer.hpp"

int main() {
    auto vert_src = GLSL(330 core,
        uniform vec2 window_size;
        uniform vec2 texture_size;

        uniform vec3 transform;

        layout (location = 0) in vec2 i_position;
        layout (location = 1) in vec2 i_texcoord;

        out vec2 tex_coords;

        void main() {
            vec2 pos = i_position;
            pos.x /= window_size.x;
            pos.y /= window_size.y;
            pos.x += transform.x;
            pos.y += transform.y;
            pos.x *= 2*transform.z;
            pos.y *= 2*transform.z;

            tex_coords = i_texcoord;
            tex_coords.x /= texture_size.x;
            tex_coords.y /= texture_size.y;

            gl_Position = vec4(pos.x, -pos.y, 0, 1.0);
        }
    );
    auto frag_src = GLSL(330 core,
        uniform sampler2D texture0;
        uniform float opacity;

        in vec2 tex_coords;
        out vec4 FragColor;

        void main() {
            vec4 tex_color = texture(texture0, tex_coords);
            FragColor = vec4(tex_color.rgb, tex_color.a * opacity);
        }
    );

    Window window(1280, 720, "LDtkViewer");

    Shader shader;
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

    while (window.isOpen()) {
        auto mouse_pos = window.getMousePosition();

        while (auto event = window.nextEvent()) {
            if (auto key = event->as<Event::Key>()) {
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
            else if (auto btn = event->as<Event::MouseButton>()) {
                if (btn->button == GLFW_MOUSE_BUTTON_LEFT) {
                    if (btn->action == GLFW_PRESS) {
                        camera_grabbed = true;
                        grab_pos = mouse_pos;
                    } else if (btn->action == GLFW_RELEASE) {
                        camera_grabbed = false;
                    }
                }
            }
            else if (auto move = event->as<Event::MouseMove>()) {
                if (camera_grabbed) {
                    auto dx = (grab_pos.x - move->x) / camera.getZoom();
                    auto dy = (grab_pos.y - move->y) / camera.getZoom();
                    grab_pos = {move->x, move->y};
                    camera.move(dx, dy);
                }
            }
            else if (auto scroll = event->as<Event::Scroll>()) {
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

        for (auto& layer : layers) {
            shader.setUniform("texture_size", glm::vec2(layer.getTexture().getSize()));
            shader.setUniform("opacity", layer.getOpacity());
            layer.render();
        }

        window.display();
    }

    return 0;
}
