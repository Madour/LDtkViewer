#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <LDtkLoader/World.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Camera2D.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexArray.hpp"
#include "Window.hpp"

int main() {

    auto window = Window(1280, 720, "LDtkViewer");

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
    Shader shader;
    shader.load(vert_src, frag_src);

    Texture texture;
    texture.load("../res/tileset.png");

    ldtk::World world;
    try {
        world.loadFromFile("../res/level.ldtk");
    }
    catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return -1;
    }
    const auto& level = world.getLevel("Level");
    const auto& tiles = level.getLayer("Ground").allTiles();

    std::vector<Vertex> vertices;
    vertices.reserve(tiles.size());
    for (const auto& tile : tiles) {
        for (int i = 0; i < 4; ++i) {
            Vertex vert{};
            vert.pos.x = tile.vertices[i].pos.x;
            vert.pos.y = tile.vertices[i].pos.y;
            vert.tex.x = static_cast<float>(tile.vertices[i].tex.x);
            vert.tex.y = static_cast<float>(tile.vertices[i].tex.y);
            vertices.push_back(vert);
        }
    }

    VertexArray va;
    va.copy(vertices);


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

        va.bind();
        va.render();

        window.display();
    }

    return 0;
}
