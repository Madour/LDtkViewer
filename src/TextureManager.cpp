// Created by Modar Nasser on 06/03/2022.

#include "TextureManager.hpp"

auto TextureManager::instance() -> TextureManager& {
    static TextureManager instance;
    return instance;
}

auto TextureManager::get(const std::string& name) -> Texture& {
    auto& data = instance().data;
    if (data.count(name) == 0)
        data[name].load(name);
    return instance().data.at(name);
}
