#include "imgui.h"
#include "imgui_custom.h"
#include "imgui_internal.h"

#include <iomanip>
#include <sstream>

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
    ImGui::Dummy({x, 0});
    ImGui::SameLine();
}

std::string ImGui::IDtoString(unsigned int ID) {
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(8) << std::hex << std::uppercase << ID;
    return stream.str();
}

struct InputTextCallback_UserData {
    std::string*            Str;
    ImGuiInputTextCallback  ChainCallback;
    void*                   ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data) {
    auto* user_data = (InputTextCallback_UserData*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        std::string* str = user_data->Str;
        IM_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    }
    else if (user_data->ChainCallback)
    {
        // Forward to user callback, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data{};
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextMultiline(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data{};
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputTextMultiline(label, (char*)str->c_str(), str->capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data{};
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputTextWithHint(label, hint, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}
