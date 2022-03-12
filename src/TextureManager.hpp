// Created by Modar Nasser on 06/03/2022.

#pragma once

#include <sogl/Texture.hpp>

#include <map>

class TextureManager {
public:
    TextureManager(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    static sogl::Texture& get(const std::string& name);
    static void clear();
private:
    TextureManager() = default;
    static TextureManager& instance();
    std::map<std::string, sogl::Texture> data;
};
