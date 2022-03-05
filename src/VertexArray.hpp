// Created by Modar Nasser on 05/03/2022.

#pragma once

#include <glm/glm.hpp>

#include <vector>

struct Vertex {
    glm::vec2 pos;
    glm::vec2 tex;
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void copy(std::vector<Vertex>& other);
    void push(const Vertex& vertex);

    void bind();

    void render() const;

private:
    unsigned m_vbo = 0;
    unsigned m_ibo = 0;
    unsigned m_vao = 0;

    bool m_dirty = false;

    std::vector<Vertex> m_vertices;
    std::vector<int> m_indices;
};

