#include <Translucence.hpp>

int main() {
    Application app;
    app.create(1000, 8000, "Bouncing Ball Example");
    Renderer renderer(app);
    EventSystem events(app);
    LayoutManager layoutManager(app);
    renderer.useLayout(&layoutManager);

    Rect propertiesPanel = {0, 0, 200, cast<float>(app.getHeight())};
    Rect canvas = {200, 0, cast<float>(app.getWidth()), cast<float>(app.getHeight())};

    InputField redInput;
    redInput.placeholder = "R";
    redInput.numericOnly = true;
    redInput.value = "0";
    InputField greenInput;
    greenInput.placeholder = "G";
    greenInput.numericOnly = true;
    greenInput.value = "0";
    InputField blueInput;
    blueInput.placeholder = "B";
    blueInput.numericOnly = true;
    blueInput.value = "0";

    int brushSize = 10;
    Slider brushSizeSlider;

    struct DrawPoint {
        float2 pos;
        float radius;
        SDL_Color color;
        bool isStart; // If true, don't draw line from previous point
    };
    List<DrawPoint> points;
    float2 lastMousePos = { -1, -1 };
    bool wasMouseDown = false;

    SDL_Color selectedColor;
    
    Button clearBtn;
    clearBtn.text = "Clear";

    while (app.isRunning()) {
        app.update();
        events.runEvents();
        if (Input::isKeyDown(Input::Key::ESCAPE)) app.setRunning(false);
        renderer.clearBackground(Color::White);

        float2 currentMousePos = Input::getMousePos();
        bool isMouseDown = Input::isMouseButtonDown(static_cast<uint8_t>(Input::MouseButton::LEFT));

        if (Input::isMouseHoveringRect(currentMousePos, canvas)) {
            if (isMouseDown) {
                bool isStart = !wasMouseDown;
                points.push_back({currentMousePos, cast<float>(brushSize), selectedColor, isStart});
            }
        }
        wasMouseDown = isMouseDown;

        for (size_t i = 0; i < points.size(); ++i) {
            const auto& pt = points[i];
            renderer.drawCircle({pt.pos, pt.radius}, pt.color);
            if (!pt.isStart && i > 0) {
                renderer.drawLine(points[i-1].pos, pt.pos, pt.color, static_cast<int>(pt.radius * 2));
            }
        }

        // Properties Panel
        renderer.drawRect(propertiesPanel, Color::SlateGray);
        renderer.column(propertiesPanel.w, 10, 10);
        {
            renderer.drawText("Properties:", Color::Black, 20);
            renderer.space(20);
            renderer.drawText("Color: ", Color::Silver, 15);
            renderer.drawInputField(redInput, 50, 25);
            renderer.drawInputField(greenInput, 50, 25);
            renderer.drawInputField(blueInput, 50, 25);

            // Draw a preview of the color
            selectedColor = {(Uint8)stringToInt(redInput.value), (Uint8)stringToInt(greenInput.value), (Uint8)stringToInt(blueInput.value), 255};
            renderer.space(10);
            Rect previewRect = renderer.getNextRect(50, 50);
            renderer.drawRect(previewRect, selectedColor);
            renderer.drawRectOutline(previewRect, Color::Black, 1);

            renderer.space(10);
            renderer.drawText("Brush Size: " + std::to_string(static_cast<int>(brushSizeSlider.value * 50 + 1)), Color::Silver, 15);
            renderer.drawSlider(brushSizeSlider, 150, 10);
            brushSize = static_cast<int>(brushSizeSlider.value * 50 + 1);

            renderer.space(20);
            renderer.drawButton(clearBtn, 150, 40);
            
        }

        if (clearBtn.isClicked) points.clear();

        renderer.render();
    }
    return 0;
}
