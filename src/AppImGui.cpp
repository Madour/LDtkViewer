// Created by Modar on 07/10/22.

#include "AppImGui.hpp"
#include "App.hpp"
#include "Config.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

AppImGui::AppImGui(App &app) : m_app(app) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(&app.getWindow(), true);
    ImGui_ImplOpenGL3_Init();

    auto& style = ImGui::GetStyle();

    style.WindowBorderSize = 0.f;
    style.WindowPadding = {0.f, 0.f};
    style.FrameRounding = 5.f;
    style.PopupBorderSize = 1;
    style.SelectableTextAlign = {0.5f, 0.5f};
    style.ScrollbarSize = 11.f;

    style.Colors[ImGuiCol_Text] = ImColor(colors::text_white);

    style.Colors[ImGuiCol_WindowBg] = ImColor(colors::window_bg);
    style.Colors[ImGuiCol_FrameBg] = ImColor(colors::frame_bg);

    style.Colors[ImGuiCol_FrameBgHovered] = ImColor(colors::hovered);
    style.Colors[ImGuiCol_FrameBgActive] = ImColor(colors::active);

    style.Colors[ImGuiCol_Header] = ImColor(colors::selected);
    style.Colors[ImGuiCol_HeaderHovered] = ImColor(colors::hovered);
    style.Colors[ImGuiCol_HeaderActive] = ImColor(colors::active);

    style.Colors[ImGuiCol_Tab] = ImColor(colors::tab_bg);
    style.Colors[ImGuiCol_TabHovered] = ImColor(colors::hovered);
    style.Colors[ImGuiCol_TabActive] = ImColor(colors::selected);

    style.Colors[ImGuiCol_Button] = ImColor(colors::btn_bg);
    style.Colors[ImGuiCol_ButtonHovered] = ImColor(colors::btn_hover);
    style.Colors[ImGuiCol_ButtonActive] = ImColor(colors::btn_active);
    style.Colors[ImGuiCol_CheckMark] = ImColor(colors::btn_hover);

    style.Colors[ImGuiCol_ScrollbarBg] = ImColor(colors::scrollbar_bg);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(colors::scrollbar_body);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(colors::scrollbar_hovered);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(colors::scrollbar_active);
}

void AppImGui::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    renderTabBar();
    renderLeftPanel();
    if (m_app.projectOpened()) {
        renderDepthSelector();
    }
    else {
        renderInstructions();
    }
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void AppImGui::renderTabBar() {
    ImGui::SetNextWindowSize({(float)m_app.getWindow().getSize().x-layout::left_panel_width, layout::tabs_bar_height});
    ImGui::SetNextWindowPos({layout::left_panel_width, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 10.f});
    ImGui::Begin("TabBar", nullptr, imgui_window_flags | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
    ImGui::BeginTabBar("ProjectsTabs", ImGuiTabBarFlags_AutoSelectNewTabs);

    std::map<std::string, bool> worlds_tabs;
    for (auto& [path, _] : m_app.allProjects()) {
        if (path.empty())
            continue;
        worlds_tabs[path] = true;
        auto filename = std::filesystem::path(path).filename().string();
        auto label = filename.append("##"+path);
        auto is_selected = m_app.getActiveProject().path == path;
        auto is_hovered = ImGui::HoveredItemLabel() == "TabBar/ProjectsTabs/"+label.substr(0, 56);
        if (is_selected || is_hovered) {
            ImGui::PushStyleColor(ImGuiCol_Text, colors::text_black);
        }
        if (ImGui::BeginTabItem(label.c_str(), &worlds_tabs[path])) {
            m_app.setActiveProject(m_app.allProjects().at(path));
            ImGui::EndTabItem();
        }
        if (is_selected || is_hovered) {
            ImGui::PopStyleColor();
        }
    }
    for (auto& [path, open] : worlds_tabs) {
        if (!open) {
            m_app.unloadLDtkFile(path.c_str());
        }
    }

    ImGui::EndTabBar();
    ImGui::End();
    ImGui::PopStyleVar();
}

void AppImGui::decorateImGuiExpandableScrollbar(const char* frame, const char* id,
                                                const std::function<void(AppImGui*)>& fn) {
    bool scrollbar_hovered = false;
    if (std::string(ImGui::HoveredItemLabel()) == frame + ("/" + ImGui::IDtoString(ImGui::GetID(id))) + "/#SCROLLY") {
        scrollbar_hovered = true;
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 15.f);
    }
    fn(this);
    if (scrollbar_hovered) {
        ImGui::PopStyleVar();
    }
}

void AppImGui::renderLeftPanel() {
    static bool demo_open = false;
    const auto* frame_name = "LeftPanel";
    if (demo_open)
        ImGui::ShowDemoWindow(&demo_open);

    ImGui::SetNextWindowSize({layout::left_panel_width, (float)m_app.getWindow().getSize().y});
    ImGui::SetNextWindowPos({0, 0});
    ImGui::Begin(frame_name, nullptr, imgui_window_flags);

    // Software Title + version
    ImGui::Pad(0, 3.5f);
    ImGui::TextCentered("LDtk Viewer v0.1");
    ImGui::PushStyleColor(ImGuiCol_Separator, colors::selected);
    ImGui::Pad(0, 0.5);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Separator();

    // demo window
    // ImGui::Pad(15, 18);
    // ImGui::Checkbox("Demo Window", &demo_open);
    // ImGui::TextCentered(ImGui::HoveredItemLabel().c_str());

    // Current world levels
    if (m_app.projectOpened()) {
        auto& active_project = m_app.getActiveProject();

        if (active_project.data->allWorlds().size() > 1) {
            renderLeftPanel_WorldsSelector();
        }

        ImGui::Pad(15, 18);

        decorateImGuiExpandableScrollbar(frame_name, "Levels", &AppImGui::renderLeftPanel_LevelsList);

        ImGui::Pad(15, 18);

        decorateImGuiExpandableScrollbar(frame_name, "Entities", &AppImGui::renderLeftPanel_EntitiesList);

        if (active_project.selected_entity != nullptr) {
            ImGui::Pad(15, 18);
            decorateImGuiExpandableScrollbar(frame_name, "Fields", &AppImGui::renderLeftPanel_FieldsList);

            if (active_project.selected_field != nullptr) {
                ImGui::Pad(15, 18);
                decorateImGuiExpandableScrollbar(frame_name, "FieldValue", &AppImGui::renderLeftPanel_FieldValues);
            }
        }
    }
    ImGui::End();
}

void AppImGui::renderLeftPanel_WorldsSelector() {
    auto& active_project = m_app.getActiveProject();
    ImGui::Pad(0, 14);
    ImGui::PushStyleColor(ImGuiCol_Text, colors::text_black);
    ImGui::PushStyleColor(ImGuiCol_Button, colors::selected);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5, 5});
    ImGui::SetNextItemWidth(layout::left_panel_width * 0.75f);
    ImGui::SetCursorPosX((layout::left_panel_width - layout::left_panel_width*0.75f) * 0.5f);
    if (ImGui::BeginCombo("##WorldsSelect", nullptr, ImGuiComboFlags_CustomPreview)) {
        for (const auto& world : active_project.objects->worlds) {
            bool is_selected = active_project.selected_world == &world;
            if (ImGui::Selectable(("##"+world.data.getName()).c_str(), is_selected)) {
                active_project.selected_world = &world;
                active_project.selected_level = &world.levels.at(0)[0];
                active_project.selected_entity = nullptr;
                active_project.selected_field = nullptr;
            }
            ImGui::SameLine();
            if (is_selected || ImGui::IsItemHovered())
                ImGui::TextCenteredColored(colors::text_black, world.data.getName().c_str());
            else
                ImGui::TextCenteredColored(colors::text_white, world.data.getName().c_str());
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    if (ImGui::BeginComboPreview()) {
        if (ImGui::IsItemHovered()) {
            ImGui::TextCenteredColored(colors::text_black, active_project.selected_world->data.getName().c_str());
        } else {
            ImGui::TextCenteredColored(colors::text_white, active_project.selected_world->data.getName().c_str());
        }
        ImGui::EndComboPreview();
    }
}

void AppImGui::renderLeftPanel_LevelsList() {
    auto& active_project = m_app.getActiveProject();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Levels");
    ImGui::BeginListBox("Levels", {layout::left_panel_width, ImGui::GetTextLineHeightWithSpacing() * 6.75f});

    for (const auto& level : active_project.selected_world->levels.at(active_project.depth)) {
        bool is_selected = active_project.selected_level == &level;
        ImGui::Selectable(("##"+level.data.iid.str()).c_str(), is_selected, ImGuiSelectableFlags_AllowItemOverlap);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            active_project.selected_level = &level;
            auto level_center = level.bounds.pos + level.bounds.size / 2.f;
            m_app.getCamera().centerOn(level_center.x, level_center.y);
            active_project.selected_entity = nullptr;
            active_project.selected_field = nullptr;
        }
        ImGui::SameLine();
        if (is_selected || ImGui::IsItemHovered())
            ImGui::TextCenteredColored(colors::text_black, level.data.name.c_str());
        else
            ImGui::TextCenteredColored(colors::text_white, level.data.name.c_str());
    }

    ImGui::EndListBox();
}

void AppImGui::renderLeftPanel_EntitiesList() {
    auto& active_project = m_app.getActiveProject();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Entities");
    ImGui::SameLine(layout::left_panel_width - 60);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 1});
    if (ImGui::Button(active_project.render_entities ? "Hide" : "Show", {50, ImGui::GetTextLineHeightWithSpacing()})) {
        active_project.render_entities = !active_project.render_entities;
    }
    ImGui::PopStyleVar();
    if (active_project.render_entities) {
        ImGui::BeginListBox("Entities", {layout::left_panel_width, ImGui::GetTextLineHeightWithSpacing() * 6.75f});

        if (active_project.selected_level != nullptr) {
            const auto& level = *active_project.selected_level;
            for (const auto& layer : level.layers) {
                for (const auto& entity : layer.entities) {
                    auto is_selected = active_project.selected_entity == &entity;
                    ImGui::Selectable(("##" + entity.data.iid.str()).c_str(), is_selected);
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                        auto posx = entity.bounds.pos.x + entity.bounds.size.x * 0.5f;
                        auto posy = entity.bounds.pos.y + entity.bounds.size.y * 0.5f;
                        active_project.selected_entity = &entity;
                        active_project.selected_field = nullptr;
                        m_app.getCamera().centerOn(static_cast<float>(posx), static_cast<float>(posy));
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("%s", entity.data.iid.str().c_str());
                    }
                    ImGui::SameLine();
                    if (is_selected || ImGui::IsItemHovered())
                        ImGui::TextCenteredColored(colors::text_black, entity.data.getName().c_str());
                    else
                        ImGui::TextCenteredColored(colors::text_white, entity.data.getName().c_str());
                }
            }
        }

        ImGui::EndListBox();
    }
    else {
        active_project.selected_entity = nullptr;
        active_project.selected_field = nullptr;
    }
}

void AppImGui::renderLeftPanel_FieldsList() {
    auto& active_project = m_app.getActiveProject();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Fields");
    ImGui::BeginListBox("Fields", {layout::left_panel_width, ImGui::GetTextLineHeightWithSpacing() * 6.75f});

    for (const auto& field : active_project.selected_entity->fields) {
        auto is_selected = active_project.selected_field == &field;
        ImGui::Selectable(("##" + std::to_string(int(field.data.type)) + " " + field.data.name).c_str(), is_selected);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            active_project.selected_field = &field;
            active_project.selected_field_values = LDtkProject::fieldValuesToString(field.data, active_project.selected_entity->data);
        }
        ImGui::SameLine();
        if (is_selected || ImGui::IsItemHovered())
            ImGui::TextCenteredColored(colors::text_black, field.data.name.c_str());
        else
            ImGui::TextCenteredColored(colors::text_white, field.data.name.c_str());
    }

    ImGui::EndListBox();
}

void AppImGui::renderLeftPanel_FieldValues() {
    auto& active_project = m_app.getActiveProject();
    const auto& field = active_project.selected_field->data;
    const auto& values = active_project.selected_field_values;

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", (LDtkProject::fieldTypeEnumToString(field.type) + " field").c_str());
    if (!LDtkProject::fieldTypeIsArray(field.type)) {
        auto height = ImGui::CalcTextSize(values.at(0).c_str()).y + ImGui::GetStyle().ItemSpacing.y;
        ImGui::BeginChildFrame(ImGui::GetID("FieldValue"), ImVec2(layout::left_panel_width, height + ImGui::GetStyle().FramePadding.y));
        ImGui::TextCentered(values.at(0).c_str());
        ImGui::EndChildFrame();
    }
    else {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {3, ImGui::GetStyle().FramePadding.y});
        ImGui::BeginChildFrame(ImGui::GetID("FieldValue"), ImVec2(layout::left_panel_width, ImGui::GetTextLineHeightWithSpacing()*6.5f));
        int i = 0;
        for (const auto& val : values) {
            auto height = ImGui::CalcTextSize(val.c_str()).y + ImGui::GetStyle().ItemSpacing.y;
            ImGui::BeginChildFrame(ImGui::GetID(std::to_string(i++).c_str()), ImVec2(layout::left_panel_width-7, height + ImGui::GetStyle().FramePadding.y));
            ImGui::TextCentered(val.c_str());
            ImGui::EndChildFrame();
        }
        ImGui::EndChildFrame();
        ImGui::PopStyleVar();
    }
}

void AppImGui::renderDepthSelector() {
    constexpr auto imgui_window_w = 45;

    auto& active_project = m_app.getActiveProject();
    auto& world = *active_project.selected_world;

    if (world.levels.size() > 1) {
        auto line_height = ImGui::GetTextLineHeightWithSpacing();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.f, 10.f});
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.f);
        ImGui::SetNextWindowSize({45, 20.f + static_cast<float>(world.levels.size()) * line_height});
        ImGui::SetNextWindowPos({layout::left_panel_width + 15, layout::tabs_bar_height + 15});
        ImGui::Begin("DepthSelector", nullptr, imgui_window_flags);

        for (auto it = world.levels.rbegin(); it != world.levels.rend(); it++) {
            const auto& [depth, _] = *it;
            ImGui::Selectable(("##"+std::to_string(depth)).c_str(), active_project.depth == depth);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                active_project.depth = depth;
                active_project.selected_level = &world.levels.at(depth)[0];
            }
            ImGui::SameLine();
            if (active_project.depth == depth || ImGui::IsItemHovered())
                ImGui::TextCenteredColored(colors::text_black, std::to_string(depth).c_str());
            else
                ImGui::TextCenteredColored(colors::text_white, std::to_string(depth).c_str());
        }
        ImGui::End();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
    }
}

void AppImGui::renderInstructions() {
    constexpr auto imgui_window_w = 400;
    constexpr auto imgui_window_h = 200;
    const auto window_posx = layout::left_panel_width + (m_app.getWindow().getSize().x - layout::left_panel_width - imgui_window_w) / 2;
    const auto window_posy = layout::tabs_bar_height + (m_app.getWindow().getSize().y - layout::tabs_bar_height - imgui_window_h) / 2;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.f);
    ImGui::SetNextWindowSize({imgui_window_w, imgui_window_h});
    ImGui::SetNextWindowPos({static_cast<float>(window_posx), static_cast<float>(window_posy)});
    ImGui::Begin("Instructions", nullptr, imgui_window_flags);
#if defined(EMSCRIPTEN)
    ImGui::Pad(0, 80);
    ImGui::TextCentered("Drag and drop your LDtk projects");
    ImGui::TextCentered("and all the needed assets here");
    ImGui::TextCentered("(you can drop an entire folder)");
#else
    ImGui::Pad(0, 90);
    ImGui::TextCentered("Drag and drop your LDtk projects here");
#endif
    ImGui::End();
    ImGui::PopStyleVar();
}
