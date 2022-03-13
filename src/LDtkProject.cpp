// Created by Modar Nasser on 13/03/2022.

#include "LDtkProject.hpp"
#include "TextureManager.hpp"

bool LDtkProject::load(const char* path) {
    ldtk::World world;
    try {
        world.loadFromFile(path);
    } catch(std::exception& ex) {
        std::cout << ex.what() << std::endl;
        return false;
    }
    name = world.getFilePath().filename();
    auto& bg = world.getBgColor();
    bg_color = {bg.r/255.f, bg.g/255.f, bg.b/255.f};
    worlds.emplace_back(world);
    return true;
}


LDtkProject::World::World(const ldtk::World& world) {
    name = world.getFilePath().filename().substr(0, world.getFilePath().filename().find('.'));

    levels.reserve(world.allLevels().size());
    for (const auto& level : world.allLevels()) {
        levels.emplace_back(level);
    }
}

LDtkProject::World::Level::Level(const ldtk::Level& level) {
    name = level.name;
    layers.reserve(level.allLayers().size());
    for (const auto& layer : level.allLayers()) {
        if (!layer.allTiles().empty())
            layers.emplace(layers.begin(), layer);
    }
}

LDtkProject::World::Level::Layer::Layer(const ldtk::Layer& layer) {
    name = layer.getName();

    auto proj_dir = layer.level->world->getFilePath().directory();
    m_texture = &TextureManager::get(proj_dir + layer.getTileset().path);

    m_va.reserve(layer.allTiles().size() * 4);
    for (const auto& tile : layer.allTiles()) {
        if (tile.getPosition().x < 0 || tile.getPosition().x > layer.getGridSize().x * layer.getCellSize()
            || tile.getPosition().y < 0 || tile.getPosition().y > layer.getGridSize().y * layer.getCellSize())
            continue;
        auto tile_verts = tile.getVertices();
        std::array<sogl::Vertex, 4> quad {};
        for (int i = 0; i < 4; ++i) {
            quad[i].pos.x = layer.level->position.x + tile_verts[i].pos.x;
            quad[i].pos.y = layer.level->position.y + tile_verts[i].pos.y;
            quad[i].tex.x = static_cast<float>(tile_verts[i].tex.x);
            quad[i].tex.y = static_cast<float>(tile_verts[i].tex.y);
            quad[i].col = {1.f, 1.f, 1.f, layer.getOpacity()};
        }
        m_va.pushQuad(quad);
    }
}

void LDtkProject::World::Level::Layer::render(sogl::Shader& shader) const {
    shader.setUniform("texture_size", glm::vec2(m_texture->getSize()));
    m_texture->bind();
    m_va.bind();
    m_va.render();
}
