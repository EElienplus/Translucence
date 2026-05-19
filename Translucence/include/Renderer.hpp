//
// Created by Stěpán Toman on 04.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_RENDERER_HPP
#define TRANSLUCENCEWORKSPACE_RENDERER_HPP

#include "T_Core.hpp"
#include "Application.hpp"
#include "Sprite.hpp"

class Renderer {
public:
    explicit Renderer(Application& argApplication) : app(argApplication) {}

    void clearBackground(SDL_Color color);
    void render();

    // Primitives

    void drawRect(Rect rect, SDL_Color color);
    void drawRectOutline(Rect rect, SDL_Color color, int thickness);
    void drawRoundedRect(Rect rect, SDL_Color color, float radius);
    void drawRoundedRectOutline(Rect rect, SDL_Color color, float radius, int thickness);
    void drawCircle(Circle circle, SDL_Color color);
    void drawCircleOutline(Circle circle, SDL_Color color, int thickness);
    void drawTriangle(Triangle triangle, SDL_Color color);
    void drawTriangleOutline(Triangle triangle, SDL_Color color, int thickness);
    void drawShape(Shape shape, SDL_Color color);
    void drawShapeOutline(Shape shape, SDL_Color color, int thickness);
    void drawLine(float2 startPos, float2 endPos, SDL_Color color, int thickness);
    void drawTail(Tail& tail, float2 point, SDL_Color color, int thickness, bool fadeThickness = true);

    void drawBezier(float2 startPos, float2 endPos, std::vector<float2> controlPoints, SDL_Color color, int thickness);
    void drawText(std::string text, float2 pos, SDL_Color color, int textSize);
    void drawList(const std::vector<std::string>& list, float2 pos, SDL_Color color, int textSize);

    void useLayout(class LayoutManager* lm) { activeLayout = lm; }

    void row(float h = -1, float padding = 0, float spacing = -1);
    void column(float w = -1, float padding = 0, float spacing = -1);
    void end();
    void space(float amount);

    Button& drawButton(Button& params);
    Button& drawButton(Button& params, class LayoutManager& layout, float w = -1, float h = -1);
    Button& drawButton(Button& params, float w, float h); // uses activeLayout

    Slider& drawSlider(Slider& params);
    Slider& drawSlider(Slider& params, class LayoutManager& layout, float w = -1, float h = -1);
    Slider& drawSlider(Slider& params, float w, float h); // uses activeLayout

    InputField& drawInputField(InputField& params);
    InputField& drawInputField(InputField& params, class LayoutManager& layout, float w = -1, float h = -1);
    InputField& drawInputField(InputField& params, float w, float h); // uses activeLayout

    void drawAxis(float2 startPos, float2 endPos, SDL_Color color, int thickness, float startValue, float endValue, int segments, const std::string& label = "", int segmentLineHeight = 10, bool drawSegmentLabel = true, int textSize = 15);
    void drawImage(const RawImage& image, float2 pos, float scale);
    void drawSprite(const Sprite& sprite, float scale = 1);

private:
    Application& app;
    class LayoutManager* activeLayout = nullptr;
};


#endif //TRANSLUCENCEWORKSPACE_RENDERER_HPP
