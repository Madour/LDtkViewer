// Created by Modar Nasser on 13/03/2022.

#include "imconfig.h"
#include "imgui.h"

using namespace ImGui;

void ImGui::TextCentered(const char* text) {
    SetCursorPosX((GetWindowSize().x - CalcTextSize(text).x) * 0.5f);
    Text(text);
}

void ImGui::Pad(float x, float y) {
    ImGui::Dummy({0, y});
    ImGui::Dummy({0, 0});
    ImGui::SameLine(x);
}
