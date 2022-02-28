// Created by Modar Nasser on 28/02/2022.

#include "Shader.hpp"
#include "Log.hpp"

#include <GL/glew.h>

#include <iostream>
#include <string>

Shader::~Shader() {
    unload();
}

bool Shader::load(const char* vert_src, const char* frag_src) {
    if (!compile(Type::Vertex, vert_src))
        return false;

    if (!compile(Type::Fragment, frag_src))
        return false;

    m_program = glCreateProgram();

    for (const auto& [_, shader] : m_shaders)
        glAttachShader(m_program, shader);

    glLinkProgram(m_program);
    glValidateProgram(m_program);

    int link_status;
    glGetProgramiv(m_program, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE) {
        std::cerr << "Failed to link shader program :\n"
                  << get_program_info_log(m_program) << std::endl;
        unload();
        return false;
    }

    int validate_status;
    glGetProgramiv(m_program, GL_VALIDATE_STATUS, &validate_status);
    if (validate_status == GL_FALSE) {
        std::cerr << "Failed to validate shader program :\n"
                  << get_program_info_log(m_program) << std::endl;
        unload();
        return false;
    }

    for (const auto& [_, shader] : m_shaders) {
        glDetachShader(m_program, shader);
        glDeleteShader(shader);
    }
    m_shaders.clear();
    std::cout << m_program << std::endl;

    return true;
}

void Shader::bind() const {
    glUseProgram(m_program);
}

int Shader::getUniformLocation(const std::string& uniform) const {
    if (m_uniforms_location.count(uniform) == 0)
        m_uniforms_location[uniform] = glGetUniformLocation(m_program, uniform.c_str());
    return m_uniforms_location.at(uniform);
}

bool Shader::compile(Type type, const char* src) {
    auto shader = glCreateShader(static_cast<unsigned>(type));
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        std::cerr << "Failed to compile "
                  << (type == Type::Vertex ? "vertex" : "fragment")
                  << " shader :\n" << get_shader_info_log(shader) << std::endl;
        glDeleteShader(shader);
        return false;
    }
    m_shaders[type] = shader;
    return true;
}


void Shader::unload() {
    for (const auto& [_, shader] : m_shaders) {
        if (shader) glDeleteShader(shader);
    }
    if (m_program) glDeleteProgram(m_program);
    m_shaders.clear();
    m_uniforms_location.clear();
}
