//
// Created by Stěpán Toman on 04.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_EVENTSYSTEM_HPP
#define TRANSLUCENCEWORKSPACE_EVENTSYSTEM_HPP

#include "T_Core.hpp"
#include "Application.hpp"
#include <functional>
#include <vector>

using EventModule = std::function<bool(Application&, const SDL_Event&)>;


class EventSystem {
public:
    explicit EventSystem(Application& application);

    void runEvents();

    void addModule(EventModule module);
    void clearModules();

private:
    Application& application;
    std::vector<EventModule> modules;
    SDL_Event event;
};

#endif //TRANSLUCENCEWORKSPACE_EVENTSYSTEM_HPP
