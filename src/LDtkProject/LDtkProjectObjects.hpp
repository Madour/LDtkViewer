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

class LDtkProjectObjects {
public:
    struct Field {
        explicit Field(const ldtk::FieldDef& field);
        const ldtk::FieldDef& data;
    };

    struct Entity {
        explicit Entity(const ldtk::Entity& Entity);
        const ldtk::Entity& data;
        std::vector<Field> fields;
        Rect bounds;
    };

    struct Layer {
        explicit Layer(const ldtk::Layer& layer, const ldtk::FilePath& filepath);
        void render(sogl::Shader& shader, bool render_entities=false) const;
        const ldtk::Layer& data;
        std::vector<Entity> entities;
    private:
        sogl::VertexArray m_va_tiles;
        sogl::VertexArray m_va_entities;
        sogl::Texture* m_texture = nullptr;
    };

    struct Level {
        explicit Level(const ldtk::Level& level, const ldtk::FilePath& filepath);
        const ldtk::Level& data;
        std::vector<Layer> layers;
        Rect bounds;
    };

    struct World {
        explicit World(const ldtk::World& world, const ldtk::FilePath& filepath);
        const ldtk::World& data;
        std::map<int, std::vector<Level>> levels;
        std::string short_name;
    };

    std::string name;
    std::vector<World> worlds;
};
