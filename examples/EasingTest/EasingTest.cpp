#include <Translucence.hpp>

int main() {
    Application app;
    app.create(1000, 800, "Translucence - Easing Test");
    Renderer renderer(app);
    EventSystem events(app);

    Circle linearCircle = {{100, 100}, 15};
    Circle smoothCircle = {{100, 150}, 15};
    Circle smootherCircle = {{100, 200}, 15};
    Circle smoothestCircle = {{100, 250}, 15};

    Slider slider;
    slider.rect = {100, 350, 400, 5};
    slider.value = 0.0f;

    bool timerStarted = false;
    float elapsedSeconds;

    float timerValue;

    while (app.isRunning()) {
        app.update();
        events.runEvents();
        if (Input::isKeyDown(Input::Key::ESCAPE)) app.setRunning(false);
        renderer.clearBackground(Color::BgDeep);

        if(Input::isKeyPressed(Input::Key::SPACE)) { timerStarted = !timerStarted; }
        if(timerStarted) { timerValue = Math::tickCounter(elapsedSeconds, 2000, app.getDeltaTime()); }
        if(Input::isKeyPressed(Input::Key::R)) {
            timerValue = 0;
            elapsedSeconds = 0;
        }

        slider.value = timerValue / 2000;
        renderer.drawSlider(slider);
        renderer.drawText("Space to play/pause", {100, 375}, Color::White, 20);
        renderer.drawText("R to reset", {100, 400}, Color::White, 20);

        linearCircle.pos.x = Math::Ease::lerp(100, 600, slider.value);
        smoothCircle.pos.x = Math::Ease::quadLerpInOut(100, 600, slider.value);
        smootherCircle.pos.x = Math::Ease::cubicLerpInOut(100, 600, slider.value);
        smoothestCircle.pos.x = Math::Ease::quarticLerpInOut(100, 600, slider.value);

        renderer.drawCircle(linearCircle, Color::White);
        renderer.drawCircle(smoothCircle, Color::White);
        renderer.drawCircle(smootherCircle, Color::White);
        renderer.drawCircle(smoothestCircle, Color::White);

        renderer.render();
    }
    return 0;
}
