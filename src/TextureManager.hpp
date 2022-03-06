// Created by Modar Nasser on 06/03/2022.

#pragma once

#include "sogl/Texture.hpp"

#include <map>

class TextureManager {
public:
    TextureManager(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    static auto get(const std::string& name)  -> Texture&;
private:
    TextureManager() = default;
    static auto instance() -> TextureManager&;
    std::map<std::string, Texture> data;
};
