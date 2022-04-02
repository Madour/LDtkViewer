// Created by Modar Nasser on 18/03/2022.

#include "LDtkProject.hpp"

#include <sstream>

bool LDtkProject::load(const char* a_path) {
    auto* project = new ldtk::Project();
    try {
        project->loadFromFile(a_path);
    } catch(std::exception& ex) {
        std::cout << ex.what() << std::endl;
        delete project;
        return false;
    }

    data = std::unique_ptr<ldtk::Project>(project);
    path = std::string(data->getFilePath().c_str());

    drawables = std::make_unique<LDtkProjectDrawables>();
    drawables->name = data->getFilePath().filename();
    for (const auto& world : data->allWorlds())
        drawables->worlds.emplace_back(world, project->getFilePath());
    selected_world = &data->allWorlds()[0];
    selected_level = &selected_world->allLevels()[0];
    rendered_world = &drawables->worlds[0];
    return true;
}

std::string LDtkProject::fieldTypeEnumToString(const ldtk::FieldType& type) {
    switch (type) {
        case ldtk::FieldType::Int:
            return "Int";
        case ldtk::FieldType::Float:
            return "Float";
        case ldtk::FieldType::Bool:
            return "Bool";
        case ldtk::FieldType::String:
            return "String";
        case ldtk::FieldType::Color:
            return "Color";
        case ldtk::FieldType::Point:
            return "Point";
        case ldtk::FieldType::Enum:
            return "Enum";
        case ldtk::FieldType::FilePath:
            return "FilePath";
        case ldtk::FieldType::EntityRef:
            return "EntityRef";
        case ldtk::FieldType::ArrayInt:
            return "ArrayInt";
        case ldtk::FieldType::ArrayFloat:
            return "ArrayFloat";
        case ldtk::FieldType::ArrayBool:
            return "ArrayBool";
        case ldtk::FieldType::ArrayString:
            return "ArrayString";
        case ldtk::FieldType::ArrayColor:
            return "ArrayColor";
        case ldtk::FieldType::ArrayPoint:
            return "ArrayPoint";
        case ldtk::FieldType::ArrayEnum:
            return "ArrayEnum";
        case ldtk::FieldType::ArrayFilePath:
            return "ArrayFilePath";
        case ldtk::FieldType::ArrayEntityRef:
            return "ArrayEntityRef";
    }
}

bool LDtkProject::fieldTypeIsArray(const ldtk::FieldType& type) {
    switch (type) {
        case ldtk::FieldType::Int:
        case ldtk::FieldType::Float:
        case ldtk::FieldType::Bool:
        case ldtk::FieldType::String:
        case ldtk::FieldType::Color:
        case ldtk::FieldType::Point:
        case ldtk::FieldType::Enum:
        case ldtk::FieldType::FilePath:
        case ldtk::FieldType::EntityRef:
            return false;
        case ldtk::FieldType::ArrayInt:
        case ldtk::FieldType::ArrayFloat:
        case ldtk::FieldType::ArrayBool:
        case ldtk::FieldType::ArrayString:
        case ldtk::FieldType::ArrayColor:
        case ldtk::FieldType::ArrayPoint:
        case ldtk::FieldType::ArrayEnum:
        case ldtk::FieldType::ArrayFilePath:
        case ldtk::FieldType::ArrayEntityRef:
            return true;
    }
}

std::vector<std::string> LDtkProject::fieldValuesToString(const ldtk::FieldDef& def, const ldtk::Entity& entity) {
    std::stringstream stream;
    std::vector<std::string> values;
    switch (def.type) {
        case ldtk::FieldType::Int:
            stream << entity.getField<ldtk::FieldType::Int>(def.name);
            values.push_back(stream.str());
            break;
        case ldtk::FieldType::Float:
            stream << entity.getField<ldtk::FieldType::Float>(def.name);
            values.push_back(stream.str());
            break;
        case ldtk::FieldType::Bool:
            stream << std::boolalpha << entity.getField<ldtk::FieldType::Bool>(def.name);
            values.push_back(stream.str());
            break;
        case ldtk::FieldType::String:
            stream << entity.getField<ldtk::FieldType::String>(def.name);
            values.push_back(stream.str());
            break;
        case ldtk::FieldType::Color:
            stream << entity.getField<ldtk::FieldType::Color>(def.name);
            values.push_back(stream.str());
            break;
        case ldtk::FieldType::Point:
            stream << entity.getField<ldtk::FieldType::Point>(def.name);
            values.push_back(stream.str());
            break;
        case ldtk::FieldType::Enum:
            stream << entity.getField<ldtk::FieldType::Enum>(def.name);
            values.push_back(stream.str());
            break;
        case ldtk::FieldType::FilePath:
            stream << entity.getField<ldtk::FieldType::FilePath>(def.name);
            values.push_back(stream.str());
            break;
        case ldtk::FieldType::EntityRef: {
            const auto& field = entity.getField<ldtk::FieldType::EntityRef>(def.name);
            stream << entity.getName() << "->";
            if (field.is_null()) {
                stream << "null";
            } else {
                const auto& val = field.value();
                stream << val->getName();
            }
            values.push_back(stream.str());
            break;
        }
        case ldtk::FieldType::ArrayInt:
            for (const auto& field : entity.getField<ldtk::FieldType::ArrayInt>(def.name)) {
                stream << field;
                values.push_back(stream.str());
                stream.str("");
            }
            break;
        case ldtk::FieldType::ArrayFloat:
            for (const auto& field : entity.getField<ldtk::FieldType::ArrayFloat>(def.name)) {
                stream << field;
                values.push_back(stream.str());
                stream.str("");
            }
            break;
        case ldtk::FieldType::ArrayBool:
            for (const auto& field : entity.getField<ldtk::FieldType::ArrayBool>(def.name)) {
                stream << std::boolalpha << field;
                values.push_back(stream.str());
                stream.str("");
            }
            break;
        case ldtk::FieldType::ArrayString:
            for (const auto& field : entity.getField<ldtk::FieldType::ArrayString>(def.name)) {
                stream << field;
                values.push_back(stream.str());
                stream.str("");
            }
            break;
        case ldtk::FieldType::ArrayColor:
            for (const auto& field : entity.getField<ldtk::FieldType::ArrayColor>(def.name)) {
                stream << field;
                values.push_back(stream.str());
                stream.str("");
            }
            break;
        case ldtk::FieldType::ArrayPoint:
            for (const auto& field : entity.getField<ldtk::FieldType::ArrayPoint>(def.name)) {
                stream << field;
                values.push_back(stream.str());
                stream.str("");
            }
            break;
        case ldtk::FieldType::ArrayEnum:
            for (const auto& field : entity.getField<ldtk::FieldType::ArrayEnum>(def.name)) {
                stream << field;
                values.push_back(stream.str());
                stream.str("");
            }
            break;
        case ldtk::FieldType::ArrayFilePath:
            for (const auto& field : entity.getField<ldtk::FieldType::ArrayFilePath>(def.name)) {
                stream << field;
                values.push_back(stream.str());
                stream.str("");
            }
            break;
        case ldtk::FieldType::ArrayEntityRef:
            for (const auto& field : entity.getField<ldtk::FieldType::ArrayEntityRef>(def.name)) {
                stream << entity.getName() << "->";
                if (field.is_null()) {
                    stream << "null";
                } else {
                    const auto& val = field.value();
                    stream << val->getName();
                }
                values.push_back(stream.str());
                stream.str("");
            }
            break;
    }
    return values;
}
