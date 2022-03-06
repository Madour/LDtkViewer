// Created by Modar Nasser on 05/03/2022.

#pragma once

#include <filesystem>

#include <glm/glm.hpp>

class Texture {
public:
    Texture();
    void load(const std::filesystem::path& file);

    void bind() const;

    const glm::vec<2, int>& getSize() const;
    int getChannelsCount() const;

private:
    unsigned m_texture = 0;
    glm::vec<2, int> m_size = {0, 0};
    int m_chan_count = 0;
};
