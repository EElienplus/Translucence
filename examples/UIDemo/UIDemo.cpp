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
    btn3.roundRadius = 15; // Extra round

    while (app.isRunning()) {
        events.runEvents();
        renderer.clearBackground(Color::BgDeep);
        if (Input::isKeyDown(Input::Key::ESCAPE)) app.setRunning(false);

        renderer.column(300, 20, 10);
        {
            renderer.drawText("Main Menu", Color::White, 40);
            renderer.space(10);

            renderer.drawButton(btn1, 300, 50);
            if (btn1.isClickedOnce) println("Button 1 clicked!");

            renderer.drawButton(btn2, 300, 50);
            if (btn2.isClickedOnce) println("Button 2 clicked!");

            renderer.space(20);

            renderer.drawText("Settings", Color::White, 25);
            renderer.drawButton(btn3, 300, 60);
            if (btn3.isClickedOnce) println("Confirm clicked!");

            renderer.space(20);

            // Generic drawing in layout
            Rect nextArea = renderer.getNextRect(300, 40);
            renderer.drawRect(nextArea, Color::withAlpha(Color::White, 50));
            renderer.drawRectOutline(nextArea, Color::White, 2);
            renderer.drawText("Generic Area", {nextArea.x + 10, nextArea.y + 10}, Color::White, 20);
        }

        renderer.render();
    }

    return 0;
}
