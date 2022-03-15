// Created by Modar Nasser on 13/03/2022.

#pragma once

#include "Camera2D.hpp"

#include <LDtkLoader/World.hpp>

#include <memory>

struct LDtkProjectVariables {
    Camera2D camera;
    int depth = 0;
    std::unique_ptr<ldtk::World> data = nullptr;
};

