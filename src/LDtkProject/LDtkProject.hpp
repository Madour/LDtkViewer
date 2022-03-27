// Created by Modar Nasser on 13/03/2022.

#pragma once

#include "Camera2D.hpp"
#include "LDtkProjectDrawables.hpp"

#include <LDtkLoader/Project.hpp>

#include <memory>

struct LDtkProject {
private:
    struct Depth {
        inline Depth& operator=(int other) {
            m_val = other;
            return *this;
        }
        inline operator int() const {
            return m_val;
        }
    private:
        int m_val = 0;
        int m_offset = 0;
    };
public:
    bool load(const char* path);

    Camera2D camera;
    Depth depth;
    std::string focused_level;
    std::unique_ptr<ldtk::Project> ldtk_data = nullptr;
    std::unique_ptr<LDtkProjectDrawables> render_data = nullptr;
};

