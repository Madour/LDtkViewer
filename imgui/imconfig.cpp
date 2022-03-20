// Created by Modar Nasser on 13/03/2022.

#include "imconfig.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <iomanip>
#include <sstream>

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

#include <iostream>

std::string ImGui::HoveredItemLabel() {
    auto& g = *ImGui::GetCurrentContext();

    ImGuiStackTool& tool = g.DebugStackTool;
    tool.LastActiveFrame = g.FrameCount;

    std::string res;
    if (!tool.Results.empty()) {
        res += ImGui::FindWindowByID(tool.Results[0].ID)->Name;
        for (int i = 1; i < tool.Results.Size; ++i) {
            res += "/";
            res += tool.Results[i].Desc;
        }
    }

    return res;
}

void ImGui::Pad(float x, float y) {
    ImGui::Dummy({0, y});
    ImGui::Dummy({0, 0});
    ImGui::SameLine(x);
}

std::string ImGui::IDtoString(unsigned int ID) {
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(8) << std::hex << std::uppercase << ID;
    return stream.str();
}
