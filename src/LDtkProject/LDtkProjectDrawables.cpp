// Created by Modar Nasser on 13/03/2022.

#include "LDtkProjectDrawables.hpp"
#include "TextureManager.hpp"

LDtkProjectDrawables::World::World(const ldtk::World& world, const ldtk::FilePath& filepath) :
data(world) {
    short_name = filepath.filename().substr(0, filepath.filename().find('.'));
    for (const auto& level : world.allLevels()) {
        levels[level.depth].emplace_back(level, filepath);
    }
}

LDtkProjectDrawables::World::Level::Level(const ldtk::Level& level, const ldtk::FilePath& filepath) :
data(level) {
    bounds.pos.x = level.position.x;
    bounds.pos.y = level.position.y;
    bounds.size.x = level.size.x;
    bounds.size.y = level.size.y;
    layers.reserve(level.allLayers().size());
    for (const auto& layer : level.allLayers()) {
        layers.emplace_back(layer, filepath);
    }
}

LDtkProjectDrawables::World::Level::Layer::Layer(const ldtk::Layer& layer, const ldtk::FilePath& filepath) :
data(layer) {
    if (!layer.allTiles().empty()) {
        auto proj_dir = filepath.directory();
        m_texture = &TextureManager::get(proj_dir + layer.getTileset().path);
    }

    m_va.reserve(layer.allTiles().size() * 4 + layer.allEntities().size()*4);

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

    for (const auto& entity : layer.allEntities()) {
        auto size = glm::vec2(entity.getSize().x, entity.getSize().y);
        auto pos = glm::vec2(entity.getPosition().x + layer.level->position.x - size.x * entity.getPivot().x,
                             entity.getPosition().y + layer.level->position.y - size.y * entity.getPivot().y);
        auto tex = glm::vec2(-1.f, -1.f);
        auto color = glm::vec4(entity.getColor().r/255.f, entity.getColor().g/255.f,
                               entity.getColor().b/255.f, (entity.getColor().a/255.f)*0.5f);

        auto tl = sogl::Vertex{pos, tex, color};
        auto tr = sogl::Vertex{{pos.x + size.x, pos.y}, tex, color};
        auto br = sogl::Vertex{{pos.x + size.x, pos.y + size.y}, tex, color};
        auto bl = sogl::Vertex{{pos.x, pos.y + size.y}, tex, color};
        m_va.pushQuad({tl, tr, br, bl});
    }
}

void LDtkProjectDrawables::World::Level::Layer::render(sogl::Shader& shader) const {
    if (m_texture != nullptr) {
        shader.setUniform("texture_size", glm::vec2(m_texture->getSize()));
        m_texture->bind();
    } else {
        shader.setUniform("texture_size", glm::vec2(0, 0));
    }
    m_va.bind();
    m_va.render();
}
