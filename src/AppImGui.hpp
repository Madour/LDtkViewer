// Created by Modar on 07/10/22.

#pragma once

#include <imgui/imgui.h>

#include <functional>

class App;

class AppImGui {
public:
    explicit AppImGui(App& app);

    void render();

private:
    static constexpr auto imgui_window_flags = ImGuiWindowFlags_NoMove
                                               | ImGuiWindowFlags_NoResize
                                               | ImGuiWindowFlags_NoDecoration;

    App& m_app;

    void renderTabBar();
    void renderLeftPanel();
    void renderLeftPanel_WorldsSelector();
    void renderLeftPanel_LevelsList();
    void renderLeftPanel_EntitiesList();
    void renderLeftPanel_FieldsList();
    void renderLeftPanel_FieldValues();
    void renderDepthSelector();
    void renderInstructions();

    void decorateImGuiExpandableScrollbar(const char* frame, const char* id,
                                          const std::function<void(AppImGui*)>& fn);
};
