#include <Translucence.hpp>

int main() {
    Application app;
    app.create(1000, 800, "Translucence - Flickering Texture");
    Renderer renderer(app);
    EventSystem events(app);

    ParticleEmitter emitter({400, 400}, 2.0f, Color::Blue, Color::Red, 10, 2);
    emitter.emissionRate = 50.0f;
    emitter.velocityMin = {-50, -50};
    emitter.velocityMax = {50, 50};

    while (app.isRunning()) {
        float deltaTime = app.getDeltaTime();
        events.runEvents();
        if (Input::isKeyDown(Input::Key::ESCAPE)) app.setRunning(false);
        renderer.clearBackground(Color::BgDeep);

        emitter.update(deltaTime);
        renderer.drawParticles(emitter);

        renderer.render();
    }
    return 0;
}