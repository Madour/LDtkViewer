// Created by Modar Nasser on 12/03/2022.

#include "App.hpp"

#include "LDtkProject/ldtk2glm.hpp"

#include <LDtkLoader/World.hpp>

#include <filesystem>

constexpr float PANEL_WIDTH = 200.f;
constexpr float BAR_HEIGHT = 30.f;

constexpr int WINDOW_WIDTH = 1366;
constexpr int WINDOW_HEIGHT = 768;

App::App() :
m_window(WINDOW_WIDTH, WINDOW_HEIGHT, "LDtk Viewer"),
m_imgui(*this) {
    m_shader.load(vert_shader, frag_shader);
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

        m_imgui.render();

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
        ctx->app.m_imgui.render();
        ctx->app.m_window.display();
    };
    emscripten_set_main_loop_arg(main_loop, &ctx, 0, EM_TRUE);
#endif
}

auto App::getWindow() -> sogl::Window& {
    return m_window;
}

auto App::allProjects() -> std::map<std::string, LDtkProject>& {
    return m_projects;
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

void App::setActiveProject(LDtkProject &project) {
    m_selected_project = &project;
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
