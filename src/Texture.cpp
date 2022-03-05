// Created by Modar Nasser on 05/03/2022.

#include "Texture.hpp"
#include "stb_image.h"

#include <GL/glew.h>

Texture::Texture() {
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Texture::load(const std::filesystem::path& file) {
    auto* texture_data = stbi_load("../res/tileset.png", &m_size.x, &m_size.y, &m_chan_count, 0);

    if (texture_data != nullptr) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_size.x, m_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(texture_data);
    }
}

void Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

const glm::vec<2, int>& Texture::getSize() const {
    return m_size;
}

int Texture::getChannelsCount() const {
    return m_chan_count;
}
