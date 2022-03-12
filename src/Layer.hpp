// Created by Modar Nasser on 06/03/2022.

#pragma once

#include <LDtkLoader/Layer.hpp>

#include "sogl/Texture.hpp"
#include "sogl/VertexArray.hpp"

class Layer {
public:
    explicit Layer(const ldtk::Layer& layer);
    const Texture& getTexture() const;
    void render();
private:
    VertexArray m_va;
    Texture* m_texture;
};

