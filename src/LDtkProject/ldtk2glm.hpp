// Created by Modar Nasser on 17/03/2022.

#pragma once

#include <glm/glm.hpp>

#include <LDtkLoader/DataTypes.hpp>

constexpr glm::vec<2, int> ldtk2glm(const ldtk::IntPoint& pt) {
    return {pt.x, pt.y};
}

constexpr glm::vec<2, float> ldtk2glm(const ldtk::FloatPoint& pt) {
    return {pt.x, pt.y};
}

constexpr glm::vec<4, float> ldtk2glm(const ldtk::Color& col) {
    return {col.r/255.f, col.g/255.f, col.b/255.f, col.a/255.f};
}
