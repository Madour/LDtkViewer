// Created by Modar Nasser on 18/03/2022.

#include "LDtkProject.hpp"

bool LDtkProject::load(const char* path) {
    auto* world = new ldtk::World();
    try {
        world->loadFromFile(path);
    } catch(std::exception& ex) {
        std::cout << ex.what() << std::endl;
        delete world;
        return false;
    }

    ldtk_data = std::unique_ptr<ldtk::World>(world);
    render_data = std::make_unique<LDtkProjectDrawables>();
    render_data->name = ldtk_data->getFilePath().filename();
    render_data->worlds.emplace_back(*ldtk_data.get());
    focused_level = render_data->worlds[0].levels[0][0].name;
    return true;
}
