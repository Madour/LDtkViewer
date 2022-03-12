// Created by Modar Nasser on 05/03/2022.

#include "VertexArray.hpp"

#include <GL/glew.h>

VertexArray::VertexArray() {
    create();
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_ibo);
    glDeleteBuffers(1, &m_vbo);
}

VertexArray::VertexArray(const VertexArray& other) : VertexArray() {
    m_vertices = other.m_vertices;
    m_indices = other.m_indices;
    m_dirty = true;
}

VertexArray& VertexArray::operator=(const VertexArray& other) {
    create();
    m_vertices = other.m_vertices;
    m_indices = other.m_indices;
    m_dirty = true;
    return *this;
}

void VertexArray::create() {
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tex));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, col));

    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
}

void VertexArray::reserve(std::size_t n) {
    m_vertices.reserve(n);
    m_indices.reserve((n / 4) * 6);
}

void VertexArray::pushTriangle(const std::array<Vertex, 3>& vertices) {
    auto index_offset = m_vertices.size();

    for (const auto& v : vertices) {
        m_vertices.push_back(v);
    }

    m_indices.push_back(index_offset);
    m_indices.push_back(index_offset + 1);
    m_indices.push_back(index_offset + 2);

    m_dirty = true;
}

void VertexArray::pushQuad(const std::array<Vertex, 4>& vertices) {
    auto index_offset = m_vertices.size();

    for (const auto& v : vertices) {
        m_vertices.push_back(v);
    }

    m_indices.push_back(index_offset);
    m_indices.push_back(index_offset + 1);
    m_indices.push_back(index_offset + 2);
    m_indices.push_back(index_offset);
    m_indices.push_back(index_offset + 2);
    m_indices.push_back(index_offset + 3);

    m_dirty = true;
}

void VertexArray::bind() const {
    if (m_dirty) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), reinterpret_cast<float*>(const_cast<Vertex*>(m_vertices.data())), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned), const_cast<unsigned*>(m_indices.data()), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_dirty = false;
    }
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
}

void VertexArray::render() const {
    glDrawElements(GL_TRIANGLES, m_vertices.size() * 6, GL_UNSIGNED_INT, nullptr);
}
