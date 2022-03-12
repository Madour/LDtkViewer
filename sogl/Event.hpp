// Created by Modar Nasser on 05/03/2022.

#pragma once

#include <optional>
#include <variant>

class Event {
    friend class Window;
public:
    struct Key { int key; int scancode; int action; int mods; };
    struct MouseButton { int button; int action; int mods; };
    struct MouseMove { int x; int y; };
    struct Scroll { int dx; int dy; };
    struct Resize { int width; int height; };

    Event() = default;

    template <typename T>
    std::optional<T> as() {
        if (std::holds_alternative<T>(this->m_data)) {
            return std::get<T>(this->m_data);
        }
        return std::nullopt;
    }

private:
    using EventTypes = std::variant<Key, MouseButton, MouseMove, Scroll, Resize>;
    EventTypes m_data;
    Event(EventTypes);
};
