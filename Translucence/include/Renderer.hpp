#ifndef TRANSLUCENCEWORKSPACE_RENDERER_HPP
#define TRANSLUCENCEWORKSPACE_RENDERER_HPP

#include "Application.hpp"
#include "T_Core.hpp"
#include "Sprite.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include "ParticleEmitter.hpp"
#include "UiPanel.hpp"

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
    void drawLine(Line line, SDL_Color color);
    void drawTail(Tail& tail, float2 point, SDL_Color color, int thickness, bool fadeThickness = true);
    void drawBezier(float2 startPos, float2 endPos, std::vector<float2> controlPoints, SDL_Color color, int thickness);

    void drawText(std::string text, float2 pos, SDL_Color color, int textSize);
    void drawText(std::string text, SDL_Color color, int textSize, float w = -1, float h = -1);
    void drawList(const std::vector<std::string>& list, float2 pos, SDL_Color color, int textSize);

    void useLayout(class LayoutManager* lm);
    Rect getNextRect(float w, float h);
    void row(float h = -1, float padding = 0, float spacing = -1);
    void column(float w = -1, float padding = 0, float spacing = -1);
    void end();
    void space(float amount);

    void drawGrid(Rect area, int tilesX, int tilesY, SDL_Color colora, SDL_Color colorb);
    void drawGridLines(Rect area, int tilesX, int tilesY, SDL_Color color, int lineWidth);

    void drawButton(Button& params);
    void drawButton(Button& params, class LayoutManager& layout, float w = -1, float h = -1);
    void drawButton(Button& params, float w, float h);
    void drawSlider(Slider& params);
    void drawSlider(Slider& params, class LayoutManager& layout, float w = -1, float h = -1);
    void drawSlider(Slider& params, float w, float h);
    void drawInputField(InputField& params);
    void drawInputField(InputField& params, class LayoutManager& layout, float w = -1, float h = -1);
    void drawInputField(InputField& params, float w, float h);

    void drawColorPicker(ColorPicker& params);
    void drawColorPicker(ColorPicker& params, class LayoutManager& layout, float w = -1, float h = -1);
    void drawColorPicker(ColorPicker& params, float w, float h);

    void drawAxis(float2 startPos, float2 endPos, SDL_Color color, int thickness, float startValue, float endValue, int segments, const std::string& label = "", int segmentLineHeight = 10, bool drawSegmentLabel = true, int textSize = 15);
    void drawImage(const RawImage& image, float2 pos, float scale = 1.0f);
    void drawImage(const RawImage& image, float w = -1, float h = -1);
    void drawImage(const RawImage& image, Rect dst);
    void drawImageRotated(const RawImage& image, Rect dst, float angle, float2 center = {-1, -1});
    void drawSprite(const Sprite& sprite, float scale = 1);
    void drawSprite(const Sprite& sprite, float w, float h);
    void drawSprite(const Sprite& sprite, Rect dst);
    void drawNineSlice(const NineSlice& ns, Rect dst);
    void updateImage(const RawImage& image);

    void drawParticles(ParticleEmitter& particleEmitter);

    // High-level public configuration method
    void screenShake(float durationInSeconds, float intensity, float frequency);

    // void applyBloom(float intensity); // it was just testing, it's very bad
    void drawPanel(UI_Panel& panel);

    Application& getApp() const { return app; }

private:
    void ensureBloomTextures();
    void blurTexture(SDL_Texture* target, SDL_Texture* temp, int iterations);
    void drawThickLine(float2 start, float2 end, SDL_Color color, float thickness);
    struct ProgressiveTexture {
        Texture texture;
        int currentY = 0;
    };

    // Screenshake state machine update methods
    void applyScreenShake(float dt);
    void resetScreenShake();

    Application& app;
    class LayoutManager* activeLayout;
    void* focusedField = nullptr; // Track which InputField has focus
    std::unordered_map<uint64_t, ProgressiveTexture> progressiveTextures;

    float shakeTimer = 0.0f;
    float shakeIntensity = 0.0f;
    float shakeFrequency = 0.0f;
    float shakeTick = 0.0f;

    SDL_Texture* sceneTexture = nullptr;
    SDL_Texture* bloomTexture1 = nullptr;
    SDL_Texture* bloomTexture2 = nullptr;
    int bloomW = 0, bloomH = 0;
};

#endif //TRANSLUCENCEWORKSPACE_RENDERER_HPP