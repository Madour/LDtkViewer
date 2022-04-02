// Created by Modar Nasser on 12/03/2022.

#include "App.hpp"
#include "Config.hpp"

#include "LDtkProject/ldtk2glm.hpp"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

#include <LDtkLoader/World.hpp>

#include <filesystem>

constexpr float PANEL_WIDTH = 200.f;
constexpr float BAR_HEIGHT = 30.f;

App::App() : m_window(1280, 720, "LDtk World Viewer") {
    m_projects.emplace("", LDtkProject{});
    m_shader.load(vert_shader, frag_shader);
    initImGui();
}

bool App::loadLDtkFile(const char* path) {
    if (m_projects.count(path) > 0) {
        unloadLDtkFile(path);
    }
    m_projects.emplace(path, LDtkProject{});
    if (m_projects.at(path).load(path)) {
        m_projects.at(path).camera.setSize(m_window.getSize());
        m_selected_project = &m_projects.at(path);
        return true;
    } else {
        m_projects.erase(path);
        return false;
    }
}

void App::unloadLDtkFile(const char* path) {
    if (m_projects.count(path)) {
        const auto selected_path = m_selected_project->path;
        m_projects.erase(path);
        if (m_projects.size() > 1) {
            if (selected_path == path)
                m_selected_project = &m_projects.rbegin()->second;
        } else {
            m_selected_project = nullptr;
        }
    }
}

void App::run() {
    while (m_window.isOpen()) {
        while (auto event = m_window.nextEvent()) {
            processEvent(event.value());
        }

        if (projectOpened()) {
            m_window.clear(ldtk2glm(getActiveProject().data->getBgColor()));
            renderActiveProject();
        } else {
            m_window.clear({54.f/255.f, 60.f/255.f, 69.f/255.f});
        }

        renderImGui();

        m_window.display();
    }
}

bool App::projectOpened() {
    return m_selected_project != nullptr;
}

void App::refreshActiveProject() {
    const auto path = m_selected_project->path;
    const auto cam = getCamera();
    const auto depth = getActiveProject().depth;
    unloadLDtkFile(path.c_str());
    loadLDtkFile(path.c_str());
    getCamera() = cam;
    getActiveProject().depth = depth;
}

LDtkProject& App::getActiveProject() {
    return *m_selected_project;
}

Camera2D& App::getCamera() {
    return m_selected_project->camera;
}

/*
void App::setActiveDepth(int depth) {
    auto& active_project_levels = getActiveProject().worlds[0].levels;
    auto depth_offset = active_project_levels.begin()->first;
    depth -= depth_offset;
    m_projects[m_selected_project].depth = (depth % active_project_levels.size()) + depth_offset;
}
*/

void App::processEvent(sogl::Event& event) {
    static bool camera_grabbed = false;
    static glm::vec<2, int> grab_pos;

    if (auto resize = event.as<sogl::Event::Resize>()) {
        for (auto& [_, data] : m_projects)
            data.camera.setSize({resize->width, resize->height});
    }
    else if (auto drop = event.as<sogl::Event::Drop>()) {
        for (auto& file : drop->files)
            loadLDtkFile(file.c_str());
    }
    else if (auto press = event.as<sogl::Event::KeyPress>()) {
        if (!ImGui::GetIO().WantCaptureKeyboard) {
            if (press->key == GLFW_KEY_ESCAPE) {
                m_window.close();
            } else if (press->key == GLFW_KEY_F5) {
                if (projectOpened()) {
                    refreshActiveProject();
                }
            }
        }
    }
    else if (auto mouse_press = event.as<sogl::Event::MousePress>()) {
        if (!ImGui::GetIO().WantCaptureMouse) {
            if (mouse_press->button == GLFW_MOUSE_BUTTON_LEFT) {
                camera_grabbed = true;
                grab_pos = m_window.getMousePosition();
            }
        }
    }
    else if (auto mouse_release = event.as<sogl::Event::MouseRelease>()) {
        if (mouse_release->button == GLFW_MOUSE_BUTTON_LEFT) {
            camera_grabbed = false;
        }
    }
    else if (auto move = event.as<sogl::Event::MouseMove>()) {
        if (camera_grabbed) {
            auto& camera = getCamera();
            auto dx = static_cast<float>(grab_pos.x - move->x) / camera.getZoom();
            auto dy = static_cast<float>(grab_pos.y - move->y) / camera.getZoom();
            grab_pos = {move->x, move->y};
            camera.move(dx, dy);
        }
    }
    else if (auto scroll = event.as<sogl::Event::Scroll>()) {
        if (!ImGui::GetIO().WantCaptureMouse) {
            auto& camera = getCamera();
            if (scroll->dy < 0) {
                camera.zoom(0.9f);
            } else if (scroll->dy > 0) {
                camera.zoom(1.1f);
            }
        }
    }
}

void App::renderActiveProject() {
    static const glm::vec2 OFFSET = {PANEL_WIDTH, BAR_HEIGHT};

    const auto& active_project = getActiveProject();

    m_shader.bind();
    m_shader.setUniform("window_size", glm::vec2(m_window.getSize()));
    m_shader.setUniform("offset", OFFSET);
    m_shader.setUniform("transform", getCamera().getTransform());

    for (const auto& world : active_project.drawables->worlds) {
        for (const auto& [depth, levels] : world.levels) {
            if (depth > active_project.depth)
                continue;
            for (const auto& level : levels) {
                if (depth == active_project.depth) {
                    auto window_size = glm::vec2(m_window.getSize());
                    auto mouse_pos = getCamera().applyTransform(glm::vec2(m_window.getMousePosition()) - OFFSET/2.f - window_size/2.f);

                    if ((mouse_pos.x >= level.bounds.pos.x && mouse_pos.y >= level.bounds.pos.y
                      && mouse_pos.x < level.bounds.pos.x + level.bounds.size.x
                      && mouse_pos.y < level.bounds.pos.y + level.bounds.size.y)
                      || level.data.name == active_project.selected_level->name) {
                        m_shader.setUniform("color", glm::vec4(1.f, 1.f, 1.f, 1.f));
                    } else {
                        m_shader.setUniform("color", glm::vec4(0.9f, 0.9f, 0.9f, 1.f));
                    }
                } else {
                    auto opacity = 0.5f - static_cast<float>(std::abs(active_project.depth - depth))/6.f;
                    m_shader.setUniform("color", glm::vec4(0.8f, 0.8f, 0.8f, opacity));
                }
                for (auto layer_it = level.layers.rbegin(); layer_it < level.layers.rend(); layer_it++)
                    layer_it->render(m_shader);
            }
        }
    }
}

void App::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(&m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    auto& style = ImGui::GetStyle();

    style.WindowBorderSize = 0.f;
    style.WindowPadding = {0.f, 0.f};
    style.FrameRounding = 5.f;
    style.SelectableTextAlign = {0.5f, 0.5f};
    style.ScrollbarSize = 11.f;

    style.Colors[ImGuiCol_Text] = ImColor(colors::text_white);

    style.Colors[ImGuiCol_WindowBg] = ImColor(colors::window_bg);
    style.Colors[ImGuiCol_FrameBg] = ImColor(colors::frame_bg);

    style.Colors[ImGuiCol_Header] = ImColor(colors::selected);
    style.Colors[ImGuiCol_HeaderHovered] = ImColor(colors::hovered);
    style.Colors[ImGuiCol_HeaderActive] = ImColor(colors::active);

    style.Colors[ImGuiCol_Tab] = ImColor(colors::tab_bg);
    style.Colors[ImGuiCol_TabHovered] = ImColor(colors::hovered);
    style.Colors[ImGuiCol_TabActive] = ImColor(colors::selected);

    style.Colors[ImGuiCol_Button] = ImColor(colors::btn_bg);
    style.Colors[ImGuiCol_ButtonHovered] = ImColor(colors::btn_hover);
    style.Colors[ImGuiCol_ButtonActive] = ImColor(colors::btn_active);

    style.Colors[ImGuiCol_ScrollbarBg] = ImColor(colors::scrollbar_bg);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(colors::scrollbar_body);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(colors::scrollbar_hovered);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(colors::scrollbar_active);
}

void App::renderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    renderImGuiTabBar();
    renderImGuiLeftPanel();
    renderImGuiDepthSelector();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::renderImGuiTabBar() {
    ImGui::SetNextWindowSize({(float)m_window.getSize().x-PANEL_WIDTH, BAR_HEIGHT});
    ImGui::SetNextWindowPos({PANEL_WIDTH, 0});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0.f, 10.f});
    ImGui::Begin("TabBar", nullptr, imgui_window_flags | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
    ImGui::BeginTabBar("ProjectsTabs", ImGuiTabBarFlags_AutoSelectNewTabs);

    std::map<std::string, bool> worlds_tabs;
    for (auto& [path, _] : m_projects) {
        if (path.empty())
            continue;
        worlds_tabs[path] = true;
        auto filename = std::filesystem::path(path).filename().string();
        auto label = filename.append("##"+path);
        auto is_selected = m_selected_project->path == path;
        auto is_hovered = ImGui::HoveredItemLabel() == "TabBar/ProjectsTabs/"+label.substr(0, 56);
        if (is_selected || is_hovered) {
            ImGui::PushStyleColor(ImGuiCol_Text, colors::text_black);
        }
        if (ImGui::BeginTabItem(label.c_str(), &worlds_tabs[path])) {
            ImGui::EndTabItem();
        }
        if (is_selected || is_hovered) {
            ImGui::PopStyleColor();
        }
        if (ImGui::IsItemActivated()) {
            m_selected_project = &m_projects.at(path);
        }
    }
    for (auto& [path, open] : worlds_tabs) {
        if (!open) {
            unloadLDtkFile(path.c_str());
        }
    }

    ImGui::EndTabBar();
    ImGui::End();
    ImGui::PopStyleVar();
}

void App::renderImGuiLeftPanel() {
    static bool demo_open = false;
    if (demo_open)
        ImGui::ShowDemoWindow(&demo_open);

    ImGui::SetNextWindowSize({PANEL_WIDTH, (float)m_window.getSize().y});
    ImGui::SetNextWindowPos({0, 0});
    ImGui::Begin("LeftPanel", nullptr, imgui_window_flags);

    // demo window
    //ImGui::Checkbox("Demo Window", &demo_open);

    // Software Title + version
    ImGui::Pad(0, 20);
    ImGui::TextCentered("LDtk Viewer v0.1");

    // Current world levels
    if (projectOpened()) {
        auto& active_project = getActiveProject();
        bool scroll_bar_hovered = false;
        ImGui::Pad(15, 18);

        if (ImGui::HoveredItemLabel() == "LeftPanel/" + ImGui::IDtoString(ImGui::GetID("Levels")) + "/#SCROLLY") {
            scroll_bar_hovered = true;
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 15.f);
        }
        ImGui::Text("Levels");
        ImGui::BeginListBox("Levels", {PANEL_WIDTH, 0});
        for (const auto& level : active_project.drawables->worlds[0].levels.at(active_project.depth)) {
            bool is_selected = active_project.selected_level->name == level.data.name;
            ImGui::Selectable(("##"+level.data.iid.str()).c_str(), is_selected, ImGuiSelectableFlags_AllowItemOverlap);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                active_project.selected_level = &level.data;
                auto level_center = level.bounds.pos + level.bounds.size / 2.f;
                getCamera().centerOn(level_center.x, level_center.y);
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
        if (scroll_bar_hovered) {
            scroll_bar_hovered = false;
            ImGui::PopStyleVar();
        }

        ImGui::Pad(15, 18);

        if (std::string(ImGui::HoveredItemLabel()) == "LeftPanel/" + ImGui::IDtoString(ImGui::GetID("Entities")) + "/#SCROLLY") {
            scroll_bar_hovered = true;
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 15.f);
        }
        ImGui::Text("Entities");
        ImGui::BeginListBox("Entities", {PANEL_WIDTH, 0});

        if (active_project.selected_level != nullptr) {
            const auto& level = *active_project.selected_level;
            for (const auto& layer : level.allLayers()) {
                for (const auto& entity : layer.allEntities()) {
                    auto is_selected = active_project.selected_entity == &entity;
                    ImGui::Selectable(("##" + entity.iid.str()).c_str(), is_selected);
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                        auto posx = entity.getPosition().x + level.position.x;
                        auto posy = entity.getPosition().y + level.position.y;
                        active_project.selected_entity = &entity;
                        active_project.selected_field = nullptr;
                        getCamera().centerOn(static_cast<float>(posx), static_cast<float>(posy));
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("%s", entity.iid.str().c_str());
                    }
                    ImGui::SameLine();
                    if (is_selected || ImGui::IsItemHovered())
                        ImGui::TextCenteredColored(colors::text_black, entity.getName().c_str());
                    else
                        ImGui::TextCenteredColored(colors::text_white, entity.getName().c_str());
                }
            }
        }
        ImGui::EndListBox();
        if (scroll_bar_hovered) {
            scroll_bar_hovered = false;
            ImGui::PopStyleVar();
        }

        ImGui::Pad(15, 18);
        static std::vector<std::string> selected_field_values;
        if (std::string(ImGui::HoveredItemLabel()) == "LeftPanel/" + ImGui::IDtoString(ImGui::GetID("Fields")) + "/#SCROLLY") {
            scroll_bar_hovered = true;
            ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 15.f);
        }
        if (active_project.selected_entity != nullptr) {
            ImGui::Text("Fields");
            ImGui::BeginListBox("Fields", {PANEL_WIDTH, 0});

            for (const auto& field : active_project.selected_entity->allFields()) {
                auto is_selected = active_project.selected_field == &field;
                ImGui::Selectable(("##" + std::to_string(int(field.type)) + " " + field.name).c_str(), is_selected);
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    active_project.selected_field = &field;
                    selected_field_values = LDtkProject::fieldValuesToString(field, *active_project.selected_entity);
                }
                ImGui::SameLine();
                if (is_selected || ImGui::IsItemHovered())
                    ImGui::TextCenteredColored(colors::text_black, field.name.c_str());
                else
                    ImGui::TextCenteredColored(colors::text_white, field.name.c_str());
            }

            ImGui::EndListBox();
            if (scroll_bar_hovered) {
                scroll_bar_hovered = false;
                ImGui::PopStyleVar();
            }

            if (active_project.selected_field != nullptr) {
                renderImGuiLeftPanel_FieldValues(*active_project.selected_field, selected_field_values);
            }
        }
    }
    ImGui::End();
}

void App::renderImGuiLeftPanel_FieldValues(const ldtk::FieldDef& field, const std::vector<std::string>& values) {
    ImGui::Pad(15, 18);
    ImGui::Text("%s", (LDtkProject::fieldTypeEnumToString(field.type) + " field").c_str());
    auto line_height = ImGui::GetTextLineHeightWithSpacing();
    if (!LDtkProject::fieldTypeIsArray(field.type)) {
        auto height = ImGui::CalcTextSize(values.at(0).c_str()).y + ImGui::GetStyle().ItemSpacing.y;
        ImGui::BeginChildFrame(ImGui::GetID("FieldValue"), ImVec2(PANEL_WIDTH, height + ImGui::GetStyle().FramePadding.y));
        ImGui::TextCentered(values.at(0).c_str());
        ImGui::EndChildFrame();
    } else {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {3, ImGui::GetStyle().FramePadding.y});
        ImGui::BeginChildFrame(ImGui::GetID("FieldValue"), ImVec2(PANEL_WIDTH, line_height*6.5f));
        int i = 0;
        for (const auto& val : values) {
            auto height = ImGui::CalcTextSize(val.c_str()).y + ImGui::GetStyle().ItemSpacing.y;
            ImGui::BeginChildFrame(ImGui::GetID(std::to_string(i++).c_str()), ImVec2(PANEL_WIDTH-7, height + ImGui::GetStyle().FramePadding.y));
            ImGui::TextCentered(val.c_str());
            ImGui::EndChildFrame();
        }
        ImGui::EndChildFrame();
        ImGui::PopStyleVar();
    }
}

void App::renderImGuiDepthSelector() {
    if (projectOpened()) {
        auto& active_project = getActiveProject();
        auto& world = active_project.drawables->worlds[0];
        if (world.levels.size() > 1) {
            auto line_height = ImGui::GetTextLineHeightWithSpacing();
            ImGui::SetNextWindowSize({45, 20.f + static_cast<float>(world.levels.size()) * line_height});
            ImGui::SetNextWindowPos({PANEL_WIDTH + 15, BAR_HEIGHT + 15});
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.f, 10.f});
            ImGui::Begin("DepthSelector", nullptr, imgui_window_flags);

            for (auto it = world.levels.rbegin(); it != world.levels.rend(); it++) {
                const auto& [depth, _] = *it;
                ImGui::Selectable(("##"+std::to_string(depth)).c_str(), active_project.depth == depth);
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                    active_project.depth = depth;
                    active_project.selected_level = &active_project.drawables->worlds[0].levels[depth][0].data;
                }
                ImGui::SameLine();
                if (active_project.depth == depth || ImGui::IsItemHovered())
                    ImGui::TextCenteredColored(colors::text_black, std::to_string(depth).c_str());
                else
                    ImGui::TextCenteredColored(colors::text_white, std::to_string(depth).c_str());
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }
    }
}
