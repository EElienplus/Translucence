#ifndef TRANSLUCENCEWORKSPACE_RENDERER_HPP
#define TRANSLUCENCEWORKSPACE_RENDERER_HPP

#include "Application.hpp"
#include "T_Core.hpp"
#include "Sprite.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include "ParticleEmitter.hpp"

class Renderer {
public:
    explicit Renderer(Application& argApplication);
    ~Renderer();

    void clearBackground(SDL_Color color);
    void render();

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

    void useLayout(class LayoutManager* lm);
    void row(float h = -1, float padding = 0, float spacing = -1);
    void column(float w = -1, float padding = 0, float spacing = -1);
    void end();
    void space(float amount);

    void drawGrid(Rect area, int tilesX, int tilesY, SDL_Color colora, SDL_Color colorb);
    void drawGridLines(Rect area, int tilesX, int tilesY, SDL_Color color, int lineWidth);

    Button& drawButton(Button& params);
    Button& drawButton(Button& params, class LayoutManager& layout, float w = -1, float h = -1);
    Button& drawButton(Button& params, float w, float h);
    Slider& drawSlider(Slider& params);
    Slider& drawSlider(Slider& params, class LayoutManager& layout, float w = -1, float h = -1);
    Slider& drawSlider(Slider& params, float w, float h);
    InputField& drawInputField(InputField& params);
    InputField& drawInputField(InputField& params, class LayoutManager& layout, float w = -1, float h = -1);
    InputField& drawInputField(InputField& params, float w, float h);

    void drawAxis(float2 startPos, float2 endPos, SDL_Color color, int thickness, float startValue, float endValue, int segments, const std::string& label = "", int segmentLineHeight = 10, bool drawSegmentLabel = true, int textSize = 15);
    void drawImage(const RawImage& image, float2 pos, float scale = 1.0f);
    void drawImage(const RawImage& image, float w = -1, float h = -1);
    void drawImage(const RawImage& image, Rect dst);
    void drawSprite(const Sprite& sprite, float scale = 1);
    void updateImage(const RawImage& image);

    void drawParticles(ParticleEmitter& particleEmitter);
private:
    struct ProgressiveTexture {
        Texture texture;
        int currentY = 0;
    };

    Application& app;
    class LayoutManager* activeLayout;
    std::unordered_map<uint64_t, ProgressiveTexture> progressiveTextures;
};

#endif //TRANSLUCENCEWORKSPACE_RENDERER_HPP