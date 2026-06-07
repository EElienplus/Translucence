#include <Translucence.hpp>
int main() {
    Application app;
    Renderer render(app);
    EventSystem events(app);

    while (app.isRunning()) {
        events.runEvents();
        render.clearBackground(Color::BgDeep);

        render.render();
    }
    return 0;
}
