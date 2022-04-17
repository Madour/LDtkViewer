// Created by Modar Nasser on 06/03/2022.

#include "TextureManager.hpp"

#define CUTE_ASEPRITE_IMPLEMENTATION
#include "thirdparty/cute_aseprite.h"

#include <iostream>
#include <filesystem>

TextureManager& TextureManager::instance() {
    static TextureManager instance;
    return instance;
}

sogl::Texture& TextureManager::get(const std::string& name) {
    auto& data = instance().data;
    if (data.count(name) == 0) {
        auto path = std::filesystem::path(name);
        if (path.extension() == ".aseprite") {
            auto* ase = cute_aseprite_load_from_file(name.c_str(), nullptr);
            auto& frame = ase->frames[0];
            if (!data[name].load(reinterpret_cast<std::uint8_t*>(frame.pixels), ase->w, ase->h, 4))
                std::cerr << "Failed to load Texture " << name << std::endl;
            cute_aseprite_free(ase);
        }
        else {
            if (!data[name].load(name))
                std::cerr << "Failed to load Texture " << name << std::endl;
        }
    }

    return instance().data.at(name);
}

void TextureManager::clear() {
    instance().data.clear();
}
