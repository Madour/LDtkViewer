// Created by Modar Nasser on 28/02/2022.

#include "Log.hpp"

#include <GL/glew.h>


std::string get_shader_info_log(unsigned shader) {
    int length;
    std::string message;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    message.resize(length);
    glGetShaderInfoLog(shader, length, nullptr, message.data());
    return message;
}

std::string get_program_info_log(unsigned program) {
    int length;
    std::string message;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    message.resize(length);
    glGetProgramInfoLog(program, length, nullptr, message.data());
    return message;
}
