// Created by Modar Nasser on 13/03/2022.

#pragma once

#include <sogl/Shader.hpp>
#include <sogl/Texture.hpp>
#include <sogl/VertexArray.hpp>

#include <LDtkLoader/Layer.hpp>
#include <LDtkLoader/Level.hpp>
#include <LDtkLoader/World.hpp>

#include <map>
#include <vector>

struct Rect {
    glm::vec2 pos;
    glm::vec2 size;
};

class LDtkProjectDrawables {
public:
    struct World {
        struct Level {
            struct Layer {
                explicit Layer(const ldtk::Layer& layer, const ldtk::FilePath& filepath);
                void render(sogl::Shader& shader) const;
                const ldtk::Layer& data;
            private:
                sogl::VertexArray m_va;
                sogl::Texture* m_texture = nullptr;
            };

            explicit Level(const ldtk::Level& level, const ldtk::FilePath& filepath);
            const ldtk::Level& data;
            Rect bounds;
            std::vector<Layer> layers;
        };

        explicit World(const ldtk::World& world, const ldtk::FilePath& filepath);
        const ldtk::World& data;
        std::string short_name;
        std::map<int, std::vector<Level>> levels;
    };

    std::string name;
    std::vector<World> worlds;
};
