// Created by Modar Nasser on 05/03/2022.

#pragma once

#include "Event.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <optional>
#include <string>
#include <list>

class Window {
    static int instance_count;

public:
    Window(int width, int height, const std::string& title);
    ~Window();

    const glm::vec<2, int>& getSize() const;

    bool isOpen() const;
    void close();

    std::optional<Event> nextEvent();
    glm::vec<2, int> getMousePosition() const;

    void clear(const glm::vec<3, float>& color = {0, 0, 0});
    void display();

private:
    GLFWwindow* m_window;
    glm::vec<2, int> m_size;
    std::list<Event> m_events;
};

