#include <array>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <LDtkLoader/World.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "sogl/stb_image.h"

#include "sogl/Shader.hpp"
#include "sogl/Texture.hpp"
#include "sogl/VertexArray.hpp"
#include "sogl/Window.hpp"

#include "Camera2D.hpp"

int main() {
    auto vert_src = GLSL(330 core,
        uniform vec2 window_size;
        uniform vec2 texture_size;

        uniform vec3 transform;

        layout (location = 0) in vec2 i_position;
        layout (location = 1) in vec2 i_texcoord;

        out vec2 tex_coords;

        void main()
        {
            vec2 pos = i_position;
            pos.x /= window_size.x;
            pos.y /= window_size.y;
            pos.x += transform.x;
            pos.y += transform.y;
            pos.x *= transform.z;
            pos.y *= transform.z;

            tex_coords = i_texcoord;
            tex_coords.x /= texture_size.x;
            tex_coords.y /= texture_size.y;

            gl_Position = vec4(pos.x, -pos.y, 0, 1.0);
        }
    );
    auto frag_src = GLSL(330 core,
        uniform sampler2D texture0;

        in vec2 tex_coords;
        out vec4 FragColor;

        void main()
        {
            FragColor = texture(texture0, tex_coords);
        }
    );

    Window window(1280, 720, "LDtkViewer");

    Shader shader;
    shader.load(vert_src, frag_src);

    Texture texture;
    texture.load("../res/SunnyLand_by_Ansimuz-extended.png");

    ldtk::World world;
    try {
        world.loadFromFile("../res/gridvania.ldtk");
    } catch(std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return 0;
    }
    const auto& level = world.getLevel("Entrance");

    std::vector<VertexArray> vertex_arrays;
    vertex_arrays.reserve(level.allLayers().size());
    for (const auto& layer : level.allLayers()) {
        if (layer.allTiles().empty()) continue;
        auto& va = vertex_arrays.emplace_back();
        va.reserve(layer.allTiles().size() * 4);
        for (const auto& tile : layer.allTiles()) {
            auto tile_verts = tile.getVertices();
            std::array<Vertex, 4> quad {};
            for (int i = 0; i < 4; ++i) {
                quad[i].pos.x = tile_verts[i].pos.x;
                quad[i].pos.y = tile_verts[i].pos.y;
                quad[i].tex.x = static_cast<float>(tile_verts[i].tex.x);
                quad[i].tex.y = static_cast<float>(tile_verts[i].tex.y);
            }
            va.pushQuad(quad);
        }
    }

    Camera2D camera(window.getSize());
    camera.centerOn(level.size.x / 2, level.size.y / 2);

    while (window.isOpen()) {
        auto mouse_pos = window.getMousePosition();

        while (auto event = window.nextEvent()) {
            if (auto key = event->as<Event::Key>()) {
                if (key->action == GLFW_PRESS) {
                    switch (key->key) {
                        case GLFW_KEY_KP_ADD:
                            camera.zoom(1.5);
                            break;
                        case GLFW_KEY_KP_SUBTRACT:
                            camera.zoom(0.5);
                            break;

                        case GLFW_KEY_LEFT:
                            camera.move(-5, 0);
                            break;
                        case GLFW_KEY_RIGHT:
                            camera.move(5, 0);
                            break;
                        case GLFW_KEY_UP:
                            camera.move(0, -5);
                            break;
                        case GLFW_KEY_DOWN:
                            camera.move(0, 5);
                            break;

                        case GLFW_KEY_ESCAPE:
                            window.close();
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (auto btn = event->as<Event::MouseButton>()) {
                if (btn->action == GLFW_PRESS) {
                    if (btn->button == GLFW_MOUSE_BUTTON_LEFT) {
                        std::cout << "mouse press " << mouse_pos.x << ", " << mouse_pos.y << std::endl;
                    }
                }
            }
        }

        window.clear();

        shader.bind();
        shader.setUniform("window_size", glm::vec2(window.getSize()));
        shader.setUniform("texture_size", glm::vec2(texture.getSize()));
        shader.setUniform("transform", camera.getTransform());

        texture.bind();

        for (auto it = vertex_arrays.rbegin(); it != vertex_arrays.rend(); it++) {
            auto& va = *it;
            va.bind();
            va.render();
        }

        window.display();
    }

    return 0;
}
