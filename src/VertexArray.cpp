// Created by Modar Nasser on 05/03/2022.

#include "VertexArray.hpp"

#include <GL/glew.h>

VertexArray::VertexArray() {
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, sizeof(Vertex), (void*)sizeof(glm::vec2));

}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_ibo);
    glDeleteBuffers(1, &m_vbo);
}

void VertexArray::copy(std::vector<Vertex>& other) {
    m_vertices.clear();
    m_vertices.resize(other.size());
    for (std::size_t i = 0; i < other.size(); ++i) {
        m_vertices[i] = other.at(i);
    }

    m_indices.clear();
    m_indices.resize(m_vertices.size() * 6);
    for (int i = 0; i < m_indices.size(); i += 6) {
        int index_offset = (i / 6) * 4;
        m_indices[i + 0] = index_offset;
        m_indices[i + 1] = index_offset + 1;
        m_indices[i + 2] = index_offset + 2;
        m_indices[i + 3] = index_offset;
        m_indices[i + 4] = index_offset + 2;
        m_indices[i + 5] = index_offset + 3;
    }

    m_dirty = true;
}

void VertexArray::push(const Vertex& vertex) {
    m_vertices.push_back(vertex);
    int index_offset = (m_indices.size() / 6) * 4;
    m_indices.push_back(index_offset);
    m_indices.push_back(index_offset + 1);
    m_indices.push_back(index_offset + 2);
    m_indices.push_back(index_offset);
    m_indices.push_back(index_offset + 2);
    m_indices.push_back(index_offset + 3);

    m_dirty = true;
}

void VertexArray::bind() {
    if (m_dirty) {
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), reinterpret_cast<float*>(m_vertices.data()), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(int), reinterpret_cast<int*>(m_indices.data()), GL_STATIC_DRAW);

        m_dirty = false;
    }
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
}

void VertexArray::render() const {
    glDrawElements(GL_TRIANGLES, m_vertices.size() * 6, GL_UNSIGNED_INT, nullptr);
}
