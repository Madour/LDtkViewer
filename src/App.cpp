// Created by Modar Nasser on 12/03/2022.

#include "App.hpp"
#include "Config.hpp"

#include "LDtkProject/ldtk2glm.hpp"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

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
    if (m_projects[path].load(path)) {
        m_projects[path].camera.setSize(m_window.getSize());
        m_selected_project = path;
        return true;
    } else {
        m_projects.erase(path);
        return false;
    }
}

void App::unloadLDtkFile(const char* path) {
    if (m_projects.count(path)) {
        m_projects.erase(path);
        m_projects.erase(path);
        m_selected_project.clear();
    }
}

void App::run() {
    while (m_window.isOpen()) {
        while (auto event = m_window.nextEvent()) {
            processEvent(event.value());
        }

        if (projectOpened()) {
            m_window.clear(ldtk2glm(getActiveProject().ldtk_data->getBgColor()));
            renderActiveProject();
        } else {
            m_window.clear({54.f/255.f, 60.f/255.f, 69.f/255.f});
        }

        renderImGui();

        m_window.display();
    }
}

bool App::projectOpened() {
    return !m_selected_project.empty();
}

void App::refreshActiveProject() {
    const auto path = m_selected_project;
    const auto cam = getCamera();
    const auto depth = getActiveProject().depth;
    unloadLDtkFile(path.c_str());
    loadLDtkFile(path.c_str());
    getCamera() = cam;
    getActiveProject().depth = depth;
}

LDtkProject& App::getActiveProject() {
    return m_projects.at(m_selected_project);
}

Camera2D& App::getCamera() {
    return m_projects.at(m_selected_project).camera;
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
            auto dx = (grab_pos.x - move->x) / camera.getZoom();
            auto dy = (grab_pos.y - move->y) / camera.getZoom();
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

    for (const auto& world : active_project.render_data->worlds) {
        for (const auto& [depth, levels] : world.levels) {
            if (depth > active_project.depth)
                continue;
            for (const auto& level : levels) {
                if (depth == active_project.depth) {
                    auto window_size = glm::vec2(m_window.getSize());
                    auto mouse_pos = getCamera().applyTransform(glm::vec2(m_window.getMousePosition()) - OFFSET/2.f - window_size/2.f);

                    if (mouse_pos.x >= level.bounds.pos.x && mouse_pos.y >= level.bounds.pos.y
                        && mouse_pos.x < level.bounds.pos.x + level.bounds.size.x
                        && mouse_pos.y < level.bounds.pos.y + level.bounds.size.y) {
                        m_shader.setUniform("color", glm::vec4(1.f, 1.f, 1.f, 1.f));
                    } else if (level.name == active_project.focused_level) {
                        m_shader.setUniform("color", glm::vec4(1.f, 1.f, 1.f, 1.f));
                    } else {
                        m_shader.setUniform("color", glm::vec4(0.9f, 0.9f, 0.9f, 1.f));
                    }
                } else {
                    m_shader.setUniform("color", glm::vec4(0.8f, 0.8f, 0.8f, 0.5f - std::abs(active_project.depth - depth)/6.f));
                }
                for (const auto& layer : level.layers)
                    layer.render(m_shader);
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
    style.ScrollbarSize = 10.f;

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
        if (ImGui::BeginTabItem(filename.c_str(), &worlds_tabs[path])) {
            if (m_selected_project != path)
                m_selected_project = path;
            ImGui::EndTabItem();
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
    ImGui::SetNextWindowSize({PANEL_WIDTH, (float)m_window.getSize().y});
    ImGui::SetNextWindowPos({0, 0});
    ImGui::Begin("LeftPanel", nullptr, imgui_window_flags);

    // Software Title + version
    ImGui::Pad(0, 20);
    ImGui::TextCentered("LDtk Viewer v0.1");

    // Current world levels
    if (projectOpened()) {
        auto& active_project = getActiveProject();
        ImGui::Pad(15, 30);

        ImGui::Text("Levels");

        ImGui::BeginListBox("Levels", {PANEL_WIDTH, 0});
        for (const auto& level : active_project.render_data->worlds[0].levels.at(active_project.depth)) {
            bool is_selected = active_project.focused_level == level.name;
            ImGui::Selectable(("##"+level.name).c_str(), is_selected, ImGuiSelectableFlags_AllowItemOverlap);
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                active_project.focused_level = level.name;
                auto level_center = level.bounds.pos + level.bounds.size / 2.f;
                getCamera().centerOn(level_center.x, level_center.y);
            }
            ImGui::SameLine();
            if (is_selected || ImGui::IsItemHovered())
                ImGui::TextCenteredColored(colors::text_black, level.name.c_str());
            else
                ImGui::TextCenteredColored(colors::text_white, level.name.c_str());
        }
        ImGui::EndListBox();

        ImGui::Pad(15, 30);

        ImGui::Text("Entities");

        ImGui::BeginListBox("Entities", {PANEL_WIDTH, 0});
        if (!active_project.focused_level.empty()) {
            const auto& level = active_project.ldtk_data->getLevel(active_project.focused_level);
            for (const auto& layer : level.allLayers()) {
                for (const auto& entity : layer.allEntities()) {
                    auto label = entity.getName() + "##" + entity.iid.c_str();
                    if (ImGui::Selectable(label.c_str(), false)) {
                        auto posx = entity.getPosition().x + level.position.x;
                        auto posy = entity.getPosition().y + level.position.y;
                        getCamera().centerOn(posx, posy);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip(entity.iid.c_str());
                    }
                }
            }
        }
        ImGui::EndListBox();
    }

    // demo window
    static bool demo_open = false;
    ImGui::Pad(15, 30);
    ImGui::Checkbox("Demo Window", &demo_open);
    ImGui::End();

    if (demo_open)
        ImGui::ShowDemoWindow(&demo_open);
}

void App::renderImGuiDepthSelector() {
    if (projectOpened()) {
        auto& active_project = getActiveProject();
        auto& world = active_project.render_data->worlds[0];
        if (world.levels.size() > 1) {
            auto line_height = ImGui::GetTextLineHeightWithSpacing();
            ImGui::SetNextWindowSize({45, 20.f + world.levels.size() * line_height});
            ImGui::SetNextWindowPos({PANEL_WIDTH + 15, BAR_HEIGHT + 15});
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.f, 10.f});
            ImGui::Begin("DepthSelector", nullptr, imgui_window_flags);

            for (auto it = world.levels.rbegin(); it != world.levels.rend(); it++) {
                const auto& [depth, _] = *it;
                if (ImGui::Selectable(std::to_string(depth).c_str(), active_project.depth == depth)) {
                    active_project.depth = depth;
                }
            }
            ImGui::End();
            ImGui::PopStyleVar();
        }
    }
}
