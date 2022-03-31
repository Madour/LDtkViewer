// Created by Modar Nasser on 18/03/2022.

#include "LDtkProject.hpp"

bool LDtkProject::load(const char* path) {
    auto* project = new ldtk::Project();
    try {
        project->loadFromFile(path);
    } catch(std::exception& ex) {
        std::cout << ex.what() << std::endl;
        delete project;
        return false;
    }

    data = std::unique_ptr<ldtk::Project>(project);
    drawables = std::make_unique<LDtkProjectDrawables>();
    drawables->name = data->getFilePath().filename();
    for (const auto& world : data->allWorlds())
        drawables->worlds.emplace_back(world, project->getFilePath());
    selected_world = &data->allWorlds()[0];
    selected_level = &selected_world->allLevels()[0];
    return true;
}
