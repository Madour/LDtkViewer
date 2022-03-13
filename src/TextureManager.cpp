// Created by Modar Nasser on 06/03/2022.

#include "TextureManager.hpp"

#include <iostream>

TextureManager& TextureManager::instance() {
    static TextureManager instance;
    return instance;
}

sogl::Texture& TextureManager::get(const std::string& name) {
    auto& data = instance().data;
    if (data.count(name) == 0)
        if (!data[name].load(name))
            std::cerr << "Failed to load Texture " << name << std::endl;
    return instance().data.at(name);
}

void TextureManager::clear() {
    instance().data.clear();
}
