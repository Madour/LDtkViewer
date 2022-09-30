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

constexpr int WINDOW_WIDTH = 1366;
constexpr int WINDOW_HEIGHT = 768;

App::App() : m_window(WINDOW_WIDTH, WINDOW_HEIGHT, "LDtk Viewer") {
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
        if (!m_projects.empty()) {
            if (selected_path == path)
                m_selected_project = &m_projects.rbegin()->second;
        } else {
            m_selected_project = nullptr;
        }
    }
}

void App::run() {
#if !defined(EMSCRIPTEN)
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
#else
    struct AppContext {
        App& app;
    };
    auto ctx = AppContext{*this};
    auto main_loop = [](void* arg) {
        auto* ctx = static_cast<AppContext*>(arg);
        while (auto event = ctx->app.m_window.nextEvent()) {
            ctx->app.processEvent(event.value());
        }
        if (ctx->app.projectOpened()) {
            ctx->app.m_window.clear(ldtk2glm(ctx->app.getActiveProject().data->getBgColor()));
            ctx->app.renderActiveProject();
        } else {
            ctx->app.m_window.clear({54.f/255.f, 60.f/255.f, 69.f/255.f});
        }
        ctx->app.renderImGui();
        ctx->app.m_window.display();
    };
    emscripten_set_main_loop_arg(main_loop, &ctx, 0, EM_TRUE);
#endif
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

void App::processEvent(sogl::Event& event) {
    static bool camera_grabbed = false;
    static glm::vec<2, int> grab_pos;

    if (auto resize = event.as<sogl::Event::Resize>()) {
        for (auto& [_, data] : m_projects) {
            data.camera.setSize({resize->width, resize->height});
        }
    }
    else if (auto drop = event.as<sogl::Event::Drop>()) {
        for (auto& file : drop->files) {
#if defined(EMSCRIPTEN)
            std::cout << "uploading file " << file << std::endl;
#endif
            std::filesystem::path filepath = file;
            if (filepath.has_extension() && filepath.extension() == ".ldtk") {
                loadLDtkFile(file.c_str());
            }
        }
    }
    else if (auto press = event.as<sogl::Event::KeyPress>()) {
        if (!ImGui::GetIO().WantCaptureKeyboard) {
            if (press->key == sogl::Key::Escape) {
                m_window.close();
            } else if (press->key == sogl::Key::F5) {
                if (projectOpened()) {
                    refreshActiveProject();
                }
            }
        }
    }
    else if (auto mouse_press = event.as<sogl::Event::MousePress>()) {
        if (!ImGui::GetIO().WantCaptureMouse) {
            if (mouse_press->button == sogl::MouseButton::Left) {
                camera_grabbed = true;
                grab_pos = m_window.getMousePosition();
            }
        }
    }
    else if (auto mouse_release = event.as<sogl::Event::MouseRelease>()) {
        if (mouse_release->button == sogl::MouseButton::Left) {
            camera_grabbed = false;
        }
    }
    else if (auto move = event.as<sogl::Event::MouseMove>()) {
        if (camera_grabbed && projectOpened()) {
            auto& camera = getCamera();
            auto dx = static_cast<float>(grab_pos.x - move->x) / camera.getZoom();
            auto dy = static_cast<float>(grab_pos.y - move->y) / camera.getZoom();
            grab_pos = {move->x, move->y};
            camera.move(dx, dy);
        }
    }
    else if (auto scroll = event.as<sogl::Event::Scroll>()) {
        if (!ImGui::GetIO().WantCaptureMouse && projectOpened()) {
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

    const auto& world = *active_project.selected_world;
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
                  || &level == active_project.selected_level) {
                    m_shader.setUniform("color", glm::vec4(1.f, 1.f, 1.f, 1.f));
                } else {
                    m_shader.setUniform("color", glm::vec4(0.9f, 0.9f, 0.9f, 1.f));
                }
            } else {
                auto opacity = 0.5f - static_cast<float>(std::abs(active_project.depth - depth))/6.f;
                m_shader.setUniform("color", glm::vec4(0.8f, 0.8f, 0.8f, opacity));
            }
            for (auto layer_it = level.layers.rbegin(); layer_it < level.layers.rend(); layer_it++)
                layer_it->render(m_shader, active_project.render_entities);
        }
    }
}

void App::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(&m_window, true);
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

void App::renderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    renderImGuiTabBar();
    renderImGuiLeftPanel();
    renderImGuiDepthSelector();
    renderImGuiInstructions();
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
            m_selected_project = &m_projects.at(path);
            ImGui::EndTabItem();
        }
        if (is_selected || is_hovered) {
            ImGui::PopStyleColor();
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
    static bool render_entities = false;
    const auto* frame_name = "LeftPanel";
    if (demo_open)
        ImGui::ShowDemoWindow(&demo_open);

    ImGui::SetNextWindowSize({PANEL_WIDTH, (float)m_window.getSize().y});
    ImGui::SetNextWindowPos({0, 0});
    ImGui::Begin(frame_name, nullptr, imgui_window_flags);

    // demo window
    // ImGui::Checkbox("Demo Window", &demo_open);
    // ImGui::TextCentered(ImGui::HoveredItemLabel().c_str());

    // Software Title + version
    ImGui::Pad(0, 3.5f);
    ImGui::TextCentered("LDtk Viewer v0.1");
    ImGui::PushStyleColor(ImGuiCol_Separator, colors::selected);
    ImGui::Pad(0, 0.5);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Separator();

    // Current world levels
    if (projectOpened()) {
        auto& active_project = getActiveProject();

        if (active_project.data->allWorlds().size() > 1) {
            renderImGuiLeftPanel_WorldsSelector();
        }

        ImGui::Pad(15, 18);

        decorateImGuiExpandableScrollbar(frame_name, "Levels", &App::renderImGuiLeftPanel_LevelsList);

        ImGui::Pad(15, 18);

        decorateImGuiExpandableScrollbar(frame_name, "Entities", &App::renderImGuiLeftPanel_EntitiesList);

        if (active_project.selected_entity != nullptr) {
            ImGui::Pad(15, 18);
            decorateImGuiExpandableScrollbar(frame_name, "Fields", &App::renderImGuiLeftPanel_FieldsList);

            if (active_project.selected_field != nullptr) {
                ImGui::Pad(15, 18);
                decorateImGuiExpandableScrollbar(frame_name, "FieldValue", &App::renderImGuiLeftPanel_FieldValues);
            }
        }
    }
    ImGui::End();
}

void App::renderImGuiLeftPanel_WorldsSelector() {
    auto& active_project = getActiveProject();
    ImGui::Pad(0, 14);
    ImGui::PushStyleColor(ImGuiCol_Text, colors::text_black);
    ImGui::PushStyleColor(ImGuiCol_Button, colors::selected);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5, 5});
    ImGui::SetNextItemWidth(PANEL_WIDTH * 0.75f);
    ImGui::SetCursorPosX((PANEL_WIDTH - PANEL_WIDTH*0.75f) * 0.5f);
    if (ImGui::BeginCombo("##WorldsSelect", nullptr, ImGuiComboFlags_CustomPreview)) {
        for (const auto& world : active_project.drawables->worlds) {
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

void App::renderImGuiLeftPanel_LevelsList() {
    auto& active_project = getActiveProject();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Levels");
    ImGui::BeginListBox("Levels", {PANEL_WIDTH, ImGui::GetTextLineHeightWithSpacing() * 6.75f});

    for (const auto& level : active_project.selected_world->levels.at(active_project.depth)) {
        bool is_selected = active_project.selected_level == &level;
        ImGui::Selectable(("##"+level.data.iid.str()).c_str(), is_selected, ImGuiSelectableFlags_AllowItemOverlap);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            active_project.selected_level = &level;
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
}

void App::renderImGuiLeftPanel_EntitiesList() {
    auto& active_project = getActiveProject();

    ImGui::AlignTextToFramePadding();
    ImGui::Text("Entities");
    ImGui::SameLine(PANEL_WIDTH - 60);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0, 1});
    if (ImGui::Button(active_project.render_entities ? "Hide" : "Show", {50, ImGui::GetTextLineHeightWithSpacing()})) {
        active_project.render_entities = !active_project.render_entities;
    }
    ImGui::PopStyleVar();
    if (active_project.render_entities) {
        ImGui::BeginListBox("Entities", {PANEL_WIDTH, ImGui::GetTextLineHeightWithSpacing() * 6.75f});

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
                        getCamera().centerOn(static_cast<float>(posx), static_cast<float>(posy));
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

void App::renderImGuiLeftPanel_FieldsList() {
    auto& active_project = getActiveProject();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Fields");
    ImGui::BeginListBox("Fields", {PANEL_WIDTH, ImGui::GetTextLineHeightWithSpacing() * 6.75f});

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

void App::renderImGuiLeftPanel_FieldValues() {
    auto& active_project = getActiveProject();
    const auto& field = active_project.selected_field->data;
    const auto& values = active_project.selected_field_values;

    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", (LDtkProject::fieldTypeEnumToString(field.type) + " field").c_str());
    if (!LDtkProject::fieldTypeIsArray(field.type)) {
        auto height = ImGui::CalcTextSize(values.at(0).c_str()).y + ImGui::GetStyle().ItemSpacing.y;
        ImGui::BeginChildFrame(ImGui::GetID("FieldValue"), ImVec2(PANEL_WIDTH, height + ImGui::GetStyle().FramePadding.y));
        ImGui::TextCentered(values.at(0).c_str());
        ImGui::EndChildFrame();
    }
    else {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {3, ImGui::GetStyle().FramePadding.y});
        ImGui::BeginChildFrame(ImGui::GetID("FieldValue"), ImVec2(PANEL_WIDTH, ImGui::GetTextLineHeightWithSpacing()*6.5f));
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

void App::decorateImGuiExpandableScrollbar(const char* frame, const char* id, const std::function<void(App*)>& fn) {
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

void App::renderImGuiDepthSelector() {
    if (projectOpened()) {
        auto& active_project = getActiveProject();
        auto& world = *active_project.selected_world;
        if (world.levels.size() > 1) {
            auto line_height = ImGui::GetTextLineHeightWithSpacing();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.f, 10.f});
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.f);
            ImGui::SetNextWindowSize({45, 20.f + static_cast<float>(world.levels.size()) * line_height});
            ImGui::SetNextWindowPos({PANEL_WIDTH + 15, BAR_HEIGHT + 15});
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
}

void App::renderImGuiInstructions() {
    if (!projectOpened()) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.f);
        ImGui::SetNextWindowSize({400, 200});
        ImGui::SetNextWindowPos({PANEL_WIDTH + (static_cast<float>(m_window.getSize().x) - PANEL_WIDTH - 400) / 2,
                                 BAR_HEIGHT + (static_cast<float>(m_window.getSize().y) - BAR_HEIGHT - 200) / 2});
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
}
