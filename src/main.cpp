#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <LDtkLoader/World.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Shader.hpp"
#include "Texture.hpp"

void error_callback(int error, const char* description) {
    std::cerr << "Error " << error << ": " << description << std::endl;
}

void GLAPIENTRY gl_debug_msg_cb(GLenum source, GLenum type, GLuint id, GLenum severity,
                                GLsizei length, const GLchar* message, const void* param) {
    std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "")
              << "id = " << id << ", source = " << source << ", type = " << type << ", severity = " << severity
              << "\n" << message << "\n" << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_E:
                std::cout << "E pressed\n";
                break;
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            default:
                break;
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            std::cout << "mouse press " << x << " , " << y << std::endl;
        }
    }
}

void window_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

struct Vertex {
    glm::vec2 pos;
    glm::vec2 tex;
};

int main() {
    glfwSetErrorCallback(error_callback);
    glfwInit();
    glm::vec2 window_size = {1280, 720};
    auto* window = glfwCreateWindow(window_size.x, window_size.y, "My Title", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, true);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, true);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize glew" << std::endl;
        return -1;
    }
    std::cout << "OpenGL version " << glGetString(GL_VERSION) << std::endl;
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_debug_msg_cb, nullptr);

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
            vert.pos.x = tile.vertices[i].pos.x / window_size.x - level.size.x / (2.f * window_size.x);
            vert.pos.y = -tile.vertices[i].pos.y / window_size.y + level.size.y / (2.f * window_size.y);
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

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        texture.bind();
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

        shader.bind();

        glDrawElements(GL_TRIANGLES, vertices.size() * 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
