// EventSystem.cpp
#include "EventSystem.hpp"

#include "Input.hpp"

EventSystem::EventSystem(Application& application) : application(application) {
}

void EventSystem::runEvents() {
	Input::beginFrame();
    application.update();

	while (SDL_PollEvent(&event)) {

		if (event.type == SDL_EVENT_QUIT) {
			application.setRunning(false);
			continue;
		}

		Input::processEvent(event);

		for (const auto &module : modules) {
			if (module(application, event)) {
				break;
			}
		}
	}

}

void EventSystem::addModule(EventModule module) {
	modules.emplace_back(std::move(module));
}

void EventSystem::clearModules() {
	modules.clear();
}
