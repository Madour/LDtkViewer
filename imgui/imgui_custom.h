#pragma once

#include <string>

namespace ImGui {
    void TextCentered(const char* text, float width=0.f);
    void TextCenteredColored(unsigned color, const char* text, float width=0.f);

    std::string HoveredItemLabel();

    void Pad(float x, float y);

    std::string IDtoString(unsigned int ID);

    // ImGui::InputText() with std::string
    // Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
    bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    bool InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
}
