// Created by Modar Nasser on 06/03/2022.

#include "Layer.hpp"

#include "TextureManager.hpp"

#include <LDtkLoader/Level.hpp>
#include <LDtkLoader/World.hpp>

Layer::Layer(const ldtk::Layer& layer) {
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

const sogl::Texture& Layer::getTexture() const {
    return *m_texture;
}

void Layer::render() {
    m_texture->bind();
    m_va.bind();
    m_va.render();
}
