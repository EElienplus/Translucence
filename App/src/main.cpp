#include <Translucence.hpp>

int main() {
    Application app;
    app.create(1000, 800, "Translucence");
    Renderer renderer(app);
    EventSystem events(app);

    while (app.isRunning()) {
        app.update();
        events.runEvents();
        if (Input::isKeyDown(Input::Key::ESCAPE)) app.setRunning(false);
        renderer.clearBackground(Color::BgDeep);



        renderer.render();
    }
    return 0;
}
