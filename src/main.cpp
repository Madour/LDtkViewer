#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Shader.hpp"

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

int main() {
    glfwSetErrorCallback(error_callback);
    glfwInit();

    auto* window = glfwCreateWindow(640, 480, "My Title", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, true);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, true);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize glew" << std::endl;
        return -1;
    }
    std::cout << "OpenGL version " << glGetString(GL_VERSION) << std::endl;
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_debug_msg_cb, nullptr);

    auto vert_src = GLSL(330 core,
         layout(location = 0) in vec4 position;
         void main() {
             gl_Position = position;
         }
    );
    auto frag_src = GLSL(330 core,
         layout(location = 0) out vec4 color;
         uniform vec4 u_color[4];
         void main() {
             color = u_color[2];
         }
    );
    Shader shader;
    shader.load(vert_src, frag_src);

    float positions[] = {
            -0.5f,  0.5f,
            0.5f,  0.5f,
            0.5f, -0.5f,
            -0.5f, -0.5f
    };

    unsigned int indices[] = {
            0, 1, 2, 2, 3, 0
    };

    unsigned int vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);


    glBindVertexArray(0);
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glm::vec4 color = {1.f, 1.f, 1.f, 1.f};

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        shader.bind();
        shader.setUniform("u_color", {color, color, color, color});
        color.r -= 0.01f;
        if (color.r < 0)
            color.r = 1.0f;

        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
