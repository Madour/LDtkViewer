// Created by Modar Nasser on 06/03/2022.

#pragma once

#include <glm/glm.hpp>

class Camera2D {
public:
    Camera2D(const glm::vec2& size);
    void setSize(const glm::vec2& size);

    void move(float x, float y);
    void zoom(float z);

    void centerOn(float x, float y);

    glm::vec2 getCenter() const;
    float getZoom() const;

    glm::vec3 getTransform() const;

private:
    glm::vec2 m_size;
    glm::vec3 m_transform = {0., 0., 1.};
};

