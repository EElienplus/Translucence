//
// Created by Stěpán Toman on 04.05.2026.
//
#include <Translucence.hpp>

int main() {
    Application app;
    app.create(1000, 800, "Translucence Application");
    app.setResizable(true);
    Renderer renderer(app);
    EventSystem events(app);

    while (app.isRunning()) {
        events.runEvents();
        if (Input::isKeyDown(Input::Key::ESCAPE)) app.setRunning(false);

        renderer.clearBackground(Color::BgDeep);

        renderer.render();
    }

    return 0;
}
