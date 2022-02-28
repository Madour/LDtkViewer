// Created by Modar Nasser on 28/02/2022.

#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>


#define GLSL(version, src) "#version "#version"\n" #src

class Shader {
public:
    enum class Type : int {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Count
    };

    ~Shader();

    bool load(const char* vert_src, const char* frag_src);
    void bind() const;

    int getUniformLocation(const std::string& uniform) const;

    template <typename T>
    void setUniform(const std::string& uniform, const T& val);

    template <typename T, glm::length_t S>
    void setUniform(const std::string& uniform, const glm::vec<S, T, glm::defaultp>& vec);

    template <typename T, glm::length_t S>
    void setUniform(const std::string& uniform, const std::initializer_list<glm::vec<S, T, glm::defaultp>>& arr);

    template <typename T>
    void setUniform(const std::string& uniform, const std::initializer_list<T>& arr);

private:
    bool compile(Type type, const char* src);
    void unload();

    unsigned m_program =  0;
    std::unordered_map<Type, unsigned> m_shaders;
    mutable std::unordered_map<std::string, int> m_uniforms_location;
};

#include "Shader.tpp"
