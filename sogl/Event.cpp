// Created by Modar Nasser on 05/03/2022.

#include "Event.hpp"

#include <utility>

Event::Event(EventTypes data) : m_data(std::move(data))
{}