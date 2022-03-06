// Created by Modar Nasser on 05/03/2022.

#pragma once

#include <glm/glm.hpp>

#include <array>
#include <vector>

struct Vertex {
    glm::vec2 pos;
    glm::vec2 tex;
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void reserve(std::size_t n);
    void copy(std::vector<Vertex>& other);

    void pushTriangle(const std::array<Vertex, 3>& vertices);
    void pushQuad(const std::array<Vertex, 4>& vertices);

    void bind();

    void render() const;

private:
    unsigned m_vao = 0;
    unsigned m_vbo = 0;
    unsigned m_ibo = 0;

    bool m_dirty = false;

    std::vector<Vertex> m_vertices;
    std::vector<unsigned> m_indices;
};

