// Created by Modar Nasser on 06/03/2022.

#pragma once

#include <sogl/Texture.hpp>
#include <sogl/VertexArray.hpp>

#include <LDtkLoader/Layer.hpp>

class Layer {
public:
    explicit Layer(const ldtk::Layer& layer);
    const sogl::Texture& getTexture() const;
    void render();
private:
    sogl::VertexArray m_va;
    sogl::Texture* m_texture;
};

