// Created by Modar Nasser on 13/03/2022.

#include "LDtkProjectDrawables.hpp"
#include "TextureManager.hpp"

#include "ldtk2glm.hpp"

static glm::vec2 level_offset = {0, 0};
static glm::vec2 level_pos = {0, 0};

LDtkProjectDrawables::World::World(const ldtk::World& world, const ldtk::FilePath& filepath) :
data(world) {
    short_name = filepath.filename().substr(0, filepath.filename().find('.'));
    level_offset = {0, 0};
    for (const auto& level : world.allLevels()) {
        auto& last_level = levels[level.depth].emplace_back(level, filepath);
        if (world.getLayout() == ldtk::WorldLayout::LinearHorizontal) {
            level_offset.x += last_level.bounds.size.x + 10;
        }
        else if (world.getLayout() == ldtk::WorldLayout::LinearVertical) {
            level_offset.y += last_level.bounds.size.y + 10;
        }
    }
}

LDtkProjectDrawables::Level::Level(const ldtk::Level& level, const ldtk::FilePath& filepath) :
data(level) {
    bounds.pos.x = level.position.x + level_offset.x;
    bounds.pos.y = level.position.y + level_offset.y;
    level_pos = bounds.pos;
    bounds.size.x = level.size.x;
    bounds.size.y = level.size.y;
    layers.reserve(level.allLayers().size());
    for (const auto& layer : level.allLayers()) {
        layers.emplace_back(layer, filepath);
    }
}

LDtkProjectDrawables::Layer::Layer(const ldtk::Layer& layer, const ldtk::FilePath& filepath) :
data(layer) {
    if (!layer.allTiles().empty()) {
        auto proj_dir = filepath.directory();
        m_texture = &TextureManager::get(proj_dir + layer.getTileset().path);
    }

    m_va_tiles.reserve(layer.allTiles().size() * 4);

    for (const auto& tile : layer.allTiles()) {
        if (tile.getPosition().x < 0 || tile.getPosition().x > layer.getGridSize().x * layer.getCellSize()
            || tile.getPosition().y < 0 || tile.getPosition().y > layer.getGridSize().y * layer.getCellSize())
            continue;
        auto tile_verts = tile.getVertices();
        std::array<sogl::Vertex, 4> quad {};
        for (int i = 0; i < 4; ++i) {
            quad[i].pos.x = level_pos.x + tile_verts[i].pos.x;
            quad[i].pos.y = level_pos.y + tile_verts[i].pos.y;
            quad[i].tex.x = static_cast<float>(tile_verts[i].tex.x);
            quad[i].tex.y = static_cast<float>(tile_verts[i].tex.y);
            quad[i].col = {1.f, 1.f, 1.f, layer.getOpacity()};
        }
        m_va_tiles.pushQuad(quad);
    }

    entities.reserve(layer.allEntities().size());
    m_va_entities.reserve(layer.allEntities().size() * 4);

    for (const auto& entity : layer.allEntities()) {
        entities.emplace_back(entity);
        auto size = glm::vec2(ldtk2glm(entity.getSize()));
        auto pos = glm::vec2(level_pos.x + entity.getPosition().x - size.x * entity.getPivot().x,
                             level_pos.y + entity.getPosition().y - size.y * entity.getPivot().y);
        auto tex = glm::vec2(-1.f, -1.f);
        auto color = ldtk2glm(entity.getColor());
        color.a *= 0.4f;

        auto tl = sogl::Vertex{pos, tex, color};
        auto tr = sogl::Vertex{{pos.x + size.x, pos.y}, tex, color};
        auto br = sogl::Vertex{{pos.x + size.x, pos.y + size.y}, tex, color};
        auto bl = sogl::Vertex{{pos.x, pos.y + size.y}, tex, color};
        m_va_entities.pushQuad({tl, tr, br, bl});
    }
}

void LDtkProjectDrawables::Layer::render(sogl::Shader& shader, bool render_entities) const {
    if (m_texture != nullptr) {
        shader.setUniform("texture_size", glm::vec2(m_texture->getSize()));
        m_texture->bind();
    } else {
        shader.setUniform("texture_size", glm::vec2(0, 0));
    }
    m_va_tiles.bind();
    m_va_tiles.render();

    if (render_entities) {
        m_va_entities.bind();
        m_va_entities.render();
    }
}

LDtkProjectDrawables::Entity::Entity(const ldtk::Entity& entity) :
data(entity) {
    fields.reserve(entity.allFields().size());
    for (const auto& field : entity.allFields()) {
        fields.emplace_back(field);
    }
    bounds.pos = level_pos + glm::vec2(ldtk2glm(entity.getPosition()));
    bounds.size = ldtk2glm(entity.getSize());
}

LDtkProjectDrawables::Field::Field(const ldtk::FieldDef& field) : data(field)
{}
