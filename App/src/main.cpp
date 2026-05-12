//
// Created by Stěpán Toman on 04.05.2026.
//
#include <Translucence.hpp>

int main() {
    Application app;
    app.create(1000, 800, "Translucence UI Pro");
    app.setResizable(true);
    Renderer renderer(app);
    EventSystem events(app);

    while (app.isRunning()) {
        events.runEvents();
        renderer.clearBackground(Color::BgDeep);

        renderer.render();
    }

    return 0;
}
