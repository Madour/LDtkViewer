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

    ldtk_data = std::unique_ptr<ldtk::Project>(project);
    render_data = std::make_unique<LDtkProjectDrawables>();
    render_data->name = ldtk_data->getFilePath().filename();
    for (const auto& world : ldtk_data->allWorlds())
        render_data->worlds.emplace_back(world, project->getFilePath());
    focused_level = render_data->worlds[0].levels[0][0].name;
    return true;
}
