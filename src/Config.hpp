// Created by Modar Nasser on 19/03/2022.

#pragma once

#include "imgui/imgui.h"

namespace colors {
    constexpr auto transparent = IM_COL32(0, 0, 0, 0);
    constexpr auto text_black = IM_COL32(0x0f, 0x0f, 0x0f, 0xff);
    constexpr auto text_white = IM_COL32(0xef, 0xef, 0xef, 0xff);

    constexpr auto selected = IM_COL32(0xff, 0xcc, 0x00, 0xff);
    constexpr auto hovered = IM_COL32(0xff, 0xed, 0x82, 0xff);
    constexpr auto active = IM_COL32(0xff, 0xff, 0xdd, 0xff);

    constexpr auto window_bg = IM_COL32(0x16, 0x18, 0x1d, 0xf9);
    constexpr auto frame_bg = IM_COL32(0x2e, 0x33, 0x3f, 0xa8);
    constexpr auto tab_bg = IM_COL32(0x34, 0x3c, 0x4f, 0xff);

    constexpr auto btn_bg = IM_COL32(0x84, 0x84, 0x84, 0xff);
    constexpr auto btn_hover = IM_COL32(0xe6, 0x76, 0x2b, 0xff);
    constexpr auto btn_active = IM_COL32(0xe1, 0x51, 0x14, 0xff);

    constexpr auto scrollbar_bg = transparent;
    constexpr auto scrollbar_body = selected;
    constexpr auto scrollbar_hovered = hovered;
    constexpr auto scrollbar_active = active;
}

namespace window {
    constexpr auto border_size = 0.f;

    constexpr auto docked_rounding = 0.f;
    constexpr auto docked_padding = ImVec2{2.f, 2.f};

    constexpr auto pinned_rounding = 10.f;
    constexpr auto pinned_padding = ImVec2{10.f, 10.f};

    constexpr auto scrollbar_width = 11.f;
    constexpr auto scrollbar_focused_width = 15.f;
}

namespace layout {
    constexpr auto left_panel_position = ImVec2{0.f, 0.f};
    constexpr auto left_panel_width = 200;

    constexpr auto tabs_bar_position = ImVec2{left_panel_width, 0.f};
    constexpr auto tabs_bar_height = 30;

    constexpr auto depth_selector_position = ImVec2{left_panel_width + 15, tabs_bar_height + 15};
    constexpr auto depth_selector_width = 45.f;
}
