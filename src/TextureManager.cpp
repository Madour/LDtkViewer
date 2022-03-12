// Created by Modar Nasser on 06/03/2022.

#include "TextureManager.hpp"

TextureManager& TextureManager::instance() {
    static TextureManager instance;
    return instance;
}

sogl::Texture& TextureManager::get(const std::string& name) {
    auto& data = instance().data;
    if (data.count(name) == 0)
        data[name].load(name);
    return instance().data.at(name);
}

void TextureManager::clear() {
    instance().data.clear();
}
