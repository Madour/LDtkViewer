#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <LDtkLoader/World.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.hpp"
#include "Texture.hpp"
#include "Window.hpp"

struct Vertex {
    glm::vec2 pos;
    glm::vec2 tex;
};

int main() {

    auto window = Window(1280, 720, "LDtkViewer");

    auto vert_src = GLSL(330 core,
         layout (location = 0) in vec2 i_position;
         layout (location = 1) in vec2 i_texcoord;

         // uniform mat4 projection;

         out vec2 tex_coords;

         void main()
         {
             vec2 pos = i_position;
             pos.x *= 8.;
             pos.y *= 8.;
             gl_Position = vec4(pos, 0.0, 1.0);
             tex_coords = i_texcoord;
         }
    );
    auto frag_src = GLSL(330 core,
         in vec2 tex_coords;
         out vec4 FragColor;

         uniform sampler2D texture0;

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

    float scale = 1.f;
    std::vector<Vertex> vertices;
    vertices.reserve(tiles.size());
    for (const auto& tile : tiles) {
        for (int i = 0; i < 4; ++i) {
            Vertex vert{};
            vert.pos.x = tile.vertices[i].pos.x / window.getSize().x - level.size.x / (2.f * window.getSize().x);
            vert.pos.y = -tile.vertices[i].pos.y / window.getSize().y + level.size.y / (2.f * window.getSize().y);
            vert.tex.x = static_cast<float>(tile.vertices[i].tex.x) / texture.getSize().x;
            vert.tex.y = static_cast<float>(tile.vertices[i].tex.y) / texture.getSize().y;
            vertices.push_back(vert);
        }
    }

    std::vector<int> indices;
    indices.resize(tiles.size() * 6, 0);
    for (int i = 0; i < indices.size(); i += 6) {
        int index_offset = (i / 6) * 4;
        indices[i + 0] = index_offset;
        indices[i + 1] = index_offset + 1;
        indices[i + 2] = index_offset + 2;
        indices[i + 3] = index_offset;
        indices[i + 4] = index_offset + 2;
        indices[i + 5] = index_offset + 3;
    }

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), reinterpret_cast<float*>(vertices.data()), GL_STATIC_DRAW);

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), reinterpret_cast<int*>(indices.data()), GL_STATIC_DRAW);

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)sizeof(glm::vec2));

    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glm::vec4 color = {1.f, 1.f, 1.f, 1.f};

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
                if (btn->action == GLFW_PRESS) {
                    if (btn->button == GLFW_MOUSE_BUTTON_LEFT) {
                        std::cout << "mouse press " << mouse_pos.x << ", " << mouse_pos.y << std::endl;
                    }
                }
            }
        }

        window.clear();

        texture.bind();
        shader.bind();

        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(GL_TRIANGLES, vertices.size() * 6, GL_UNSIGNED_INT, nullptr);

        window.display();
    }

    return 0;
}
