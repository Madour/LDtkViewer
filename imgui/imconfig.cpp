// Created by Modar Nasser on 13/03/2022.

#include "imconfig.h"
#include "imgui.h"

using namespace ImGui;

void ImGui::TextCentered(const char* text, float width) {
    if (width == 0)
        SetCursorPosX((GetWindowSize().x - CalcTextSize(text).x) * 0.5f);
    else
        SetCursorPosX((width - CalcTextSize(text).x) * 0.5f);

    Text("%s", text);
}

void ImGui::TextCenteredColored(unsigned color, const char* text, float width) {
    if (width == 0)
        SetCursorPosX((GetWindowSize().x - CalcTextSize(text).x) * 0.5f);
    else
        SetCursorPosX((width - CalcTextSize(text).x) * 0.5f);

    TextColored(ImColor(color), "%s", text);
}

void ImGui::Pad(float x, float y) {
    ImGui::Dummy({0, y});
    ImGui::Dummy({0, 0});
    ImGui::SameLine(x);
}
