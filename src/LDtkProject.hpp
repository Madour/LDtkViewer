// Created by Modar Nasser on 13/03/2022.

#pragma once

#include <sogl/Shader.hpp>
#include <sogl/Texture.hpp>
#include <sogl/VertexArray.hpp>

#include <LDtkLoader/Layer.hpp>
#include <LDtkLoader/Level.hpp>
#include <LDtkLoader/World.hpp>

#include <vector>

class LDtkProject {
public:
    struct World {
        struct Level {
            struct Layer {
                explicit Layer(const ldtk::Layer& layer);
                void render(sogl::Shader& shader) const;
                std::string name;
            private:
                sogl::VertexArray m_va;
                sogl::Texture* m_texture;
            };

            explicit Level(const ldtk::Level& level);
            std::string name;
            std::vector<Layer> layers;
        };

        explicit World(const ldtk::World& world);
        std::string name;
        std::vector<Level> levels;
    };

    bool load(const char* path);
    std::string name;
    glm::vec3 bg_color = {0, 0, 0};
    std::vector<World> worlds;
};
