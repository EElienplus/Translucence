#include <Translucence.hpp>

int main() {
    Application app;
    app.create(1000, 800, "Translucence UI Demo");
    app.setResizable(true);
    Renderer renderer(app);
    EventSystem events(app);
    LayoutManager layout(app);
    renderer.useLayout(&layout);

    Button btn1;
    btn1.text = "Button 1";
    Button btn2;
    btn2.text = "Button 2";
    Button btn3;
    btn3.text = "Blue Button";
    btn3.bgColor = Color::Accent;

    while (app.isRunning()) {
        events.runEvents();
        renderer.clearBackground(Color::BgDeep);
        if (Input::isKeyDown(Input::Key::ESCAPE)) app.setRunning(false);

        renderer.column(300, 20, 10);
        {
            renderer.drawButton(btn1, 300, 50);
            if (btn1.isClickedOnce) println("Button 1 clicked!");

            renderer.drawButton(btn2, 300, 50);
            if (btn2.isClickedOnce) println("Button 2 clicked!");

            renderer.space(20);

            renderer.drawButton(btn3, 300, 60);
            if (btn3.isClickedOnce) println("Confirm clicked!");
        }

        renderer.render();
    }

    return 0;
}
