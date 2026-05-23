/* I actually didn't implement a camera system and I don't plan to implement one soon, maybe in the future though
 * I just made the screen shake by offsetting the sdl renderer*/

#include <Translucence.hpp>

int main() {
    Application app;
    app.create(1000, 800, "Screenshake Test");
    Renderer renderer(app);
    EventSystem events(app);

    ParticleEmitter emitter;
    {
        emitter.pos = { 500, 400 };
        emitter.velocityMin = { -300.0f, -300.0f };
        emitter.velocityMax = {  300.0f,  300.0f };
        emitter.minLifeTime = 0.3f;
        emitter.maxLifeTime = 0.7f;
        emitter.startColor = SDL_Color{ 255, 200, 50,  255 };
        emitter.endColor   = SDL_Color{ 255, 50,  0,   0 };
        emitter.startSize = 8.0f;
        emitter.endSize   = 1.0f;
        emitter.emissionRate = 0;
    } // I put it into scopes, so I can just collapse it to make the code cleaner

    while (app.isRunning()) {
        events.runEvents();

        if (Input::isKeyDown(Input::Key::ESCAPE)) {
            app.setRunning(false);
        }
        if (Input::isKeyPressed(Input::Key::SPACE)) {
            // Arguments: Duration (0.4s), Intensity (15.0 pixels), Frequency (45.0Hz)
            renderer.screenShake(0.4f, 15.0f, 45.0f);

            emitter.emit(30);
        }
        renderer.clearBackground(Color::BgDeep);

        emitter.update(app.getDeltaTime());
        renderer.drawParticles(emitter);

        renderer.drawRect({ 300, 200, 100, 100 }, Color::Red);
        renderer.drawText("Press space to shake", { 400, 150 }, Color::White, 20);

        renderer.render();
    }
    return 0;
}
