// Created by Modar Nasser on 13/03/2022.

#pragma once

#include "Camera2D.hpp"
#include "LDtkProjectObjects.hpp"

#include <LDtkLoader/Project.hpp>

#include <memory>
#include <string>
#include <vector>

struct LDtkProject {
public:
    bool load(const char* path);
    static std::string fieldTypeEnumToString(const ldtk::FieldType& type);
    static bool fieldTypeIsArray(const ldtk::FieldType& type);
    static std::vector<std::string> fieldValuesToString(const ldtk::FieldDef& def, const ldtk::Entity& entity);

    Camera2D camera;
    int depth;
    std::string path;
    bool render_entities = false;

    const LDtkProjectObjects::World* selected_world = nullptr;
    const LDtkProjectObjects::Level* selected_level = nullptr;
    const LDtkProjectObjects::Entity* selected_entity = nullptr;
    const LDtkProjectObjects::Field* selected_field = nullptr;
    std::vector<std::string> selected_field_values;

    std::unique_ptr<ldtk::Project> data = nullptr;
    std::unique_ptr<LDtkProjectObjects> objects = nullptr;
};
