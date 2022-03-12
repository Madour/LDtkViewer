// Created by Modar Nasser on 05/03/2022.

#pragma once

#include <glm/glm.hpp>

#include <array>
#include <vector>

struct Vertex {
    glm::vec2 pos;
    glm::vec2 tex;
    glm::vec4 col;
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();
    VertexArray(const VertexArray&);
    VertexArray& operator=(const VertexArray&);

    void reserve(std::size_t n);

    void pushTriangle(const std::array<Vertex, 3>& vertices);
    void pushQuad(const std::array<Vertex, 4>& vertices);

    void bind() const;
    void render() const;

private:
    void create();

    unsigned m_vao = 0;
    unsigned m_vbo = 0;
    unsigned m_ibo = 0;

    std::vector<Vertex> m_vertices;
    std::vector<unsigned> m_indices;

    mutable bool m_dirty = false;
};

