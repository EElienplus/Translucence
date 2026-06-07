//
// Created by Stěpán Toman on 04.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_CORE_HPP
#define TRANSLUCENCEWORKSPACE_CORE_HPP

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <filesystem>
#include <vector>

#define INT_LIMIT 2147483647
#define UNSIGNED_INT_LIMIT 4294967295

namespace fs = std::filesystem;
#define List std::vector
#define String std::string

template <typename T, typename T1>
T cast(T1 value) {
    return static_cast<T>(value);
}


template <typename T>
std::string toString(T value) {
    if constexpr (std::is_same_v<T, std::vector<float>>) {
        std::string res = "[";
        for (size_t i = 0; i < value.size(); ++i) {
            res += std::to_string(value[i]);
            if (i < value.size() - 1) res += ", ";
        }
        res += "]";
        return res;
    } else {
        return std::to_string(value);
    }
}

inline std::vector<String> toStringVec(std::vector<float> vec) {
    std::vector<String> returnVec{};
    for (float f : vec) {
        returnVec.push_back(toString(f));
    }
    return returnVec;
}

template <typename T>
void print(T value) {
    std::cout << value;
}

template <typename T>
void println(T value) {
    std::cout << value << "\n";
}

String askForInput(const String& askMsg) {
    print(askMsg);
    String reply;
    std::cin >> reply;
    return reply;
}

int stringToInt(String num) {
    if (num.empty()) return 0;
    try {
        return std::stoi(num);
    } catch (...) {
        return 0;
    }
}

template <typename T>
struct vec2 {
    T v0;
    T v1;
    vec2();
    vec2(T v0, T v1) {
        this->v0 = v0;
        this->v1 = v1;
    }
};

struct float2 {
    float x{}, y{};
    float2(float x, float y) {
        this->x = x;
        this->y = y;
    }

    float2() : x(0), y(0) {}

    float2 operator+(const float2& other) const { return {x + other.x, y + other.y}; }
    float2 operator-(const float2& other) const { return {x - other.x, y - other.y}; }
    float2 operator*(float s) const { return {x * s, y * s}; }
    float2 operator/(float s) const { return {x / s, y / s}; }
    float2& operator+=(const float2& other) { x += other.x; y += other.y; return *this; }
    float2& operator-=(const float2& other) { x -= other.x; y -= other.y; return *this; }
    float2& operator*=(float s) { x *= s; y *= s; return *this; }
    float2& operator/=(float s) { x /= s; y /= s; return *this; }

    [[nodiscard]] float lengthSq() const { return x * x + y * y; }
    [[nodiscard]] float length() const { return std::sqrt(x * x + y * y); }
    [[nodiscard]] float2 normalized() const {
        float len = length();
        return len > 0 ? *this / len : float2(0, 0);
    }
    static float dot(const float2& a, const float2& b) { return a.x * b.x + a.y * b.y; }
    static float2 lerp(const float2& a, const float2& b, float t) {
        return a + (b - a) * t;
    }
};

struct float3 {
    float x, y, z;
    float3(float x, float y, float z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

struct float4 {
    float x, y, z, w;
    float4(float x, float y, float z, float w) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
};


template <>
inline vec2<int> cast<vec2<int>>(float2 value) {
    return {cast<int>(value.x), cast<int>(value.y)};
}
template <>
inline float2 cast<float2>(vec2<int> value) {
    return {cast<float>(value.v0), cast<float>(value.v1)};
}

struct Color {
    // Classic
    static constexpr SDL_Color White = {255, 255, 255, 255};
    static constexpr SDL_Color Black = {0, 0, 0, 255};
    static constexpr SDL_Color Red = {255, 0, 0, 255};
    static constexpr SDL_Color Green = {0, 255, 0, 255};
    static constexpr SDL_Color Blue = {0, 0, 255, 255};
    static constexpr SDL_Color Cyan = {0, 255, 255, 255};
    static constexpr SDL_Color Magenta = {255, 0, 255, 255};
    static constexpr SDL_Color Yellow = {255, 255, 0, 255};

    // Earthy / saturated
    static constexpr SDL_Color NavyBlue = {32, 42, 68, 255};
    static constexpr SDL_Color Crimson = {220, 20, 60, 255};
    static constexpr SDL_Color Maroon = {128, 0, 0, 255};
    static constexpr SDL_Color Indigo = {75, 0, 130, 255};
    static constexpr SDL_Color Charcoal = {54, 69, 79, 255};
    static constexpr SDL_Color SlateGray = {112, 128, 144, 255};
    static constexpr SDL_Color DimGray = {105, 105, 105, 255};
    static constexpr SDL_Color DarkGray = {65, 65, 65, 255};
    static constexpr SDL_Color ForestGreen = {34, 139, 34, 255};
    static constexpr SDL_Color Teal = {0, 128, 128, 255};
    static constexpr SDL_Color Orange = {255, 165, 0, 255};
    static constexpr SDL_Color DeepPink = {255, 20, 147, 255};
    static constexpr SDL_Color Gold = {255, 215, 0, 255};
    static constexpr SDL_Color SkyBlue = {135, 206, 235, 255};
    static constexpr SDL_Color SpringGreen = {0, 255, 127, 255};
    static constexpr SDL_Color Violet = {238, 130, 238, 255};
    static constexpr SDL_Color Chocolate = {210, 105, 30, 255};
    static constexpr SDL_Color Sienna = {160, 82, 45, 255};
    static constexpr SDL_Color SaddleBrown = {139, 69, 19, 255};
    static constexpr SDL_Color Goldenrod = {218, 165, 32, 255};
    static constexpr SDL_Color Olive = {128, 128, 0, 255};
    static constexpr SDL_Color Mint = {189, 252, 201, 255};
    static constexpr SDL_Color Peach = {255, 218, 185, 255};
    static constexpr SDL_Color Lavender = {230, 230, 250, 255};
    static constexpr SDL_Color Ivory = {255, 255, 240, 255};
    static constexpr SDL_Color Silver = {192, 192, 192, 255};
    static constexpr SDL_Color Gold2 = {255, 215, 0, 255};
    static constexpr SDL_Color Silver2 = {192, 192, 192, 255};
    static constexpr SDL_Color Bronze = {205, 127, 50, 255};
    static constexpr SDL_Color Coral = {255, 127, 80, 255};
    static constexpr SDL_Color Salmon = {250, 128, 114, 255};
    static constexpr SDL_Color Khaki = {240, 230, 140, 255};
    static constexpr SDL_Color Plum = {221, 160, 221, 255};
    static constexpr SDL_Color Orchid = {218, 112, 214, 255};
    static constexpr SDL_Color Tan = {210, 180, 140, 255};
    static constexpr SDL_Color RosyBrown = {188, 143, 143, 255};
    static constexpr SDL_Color SeaGreen = {46, 139, 87, 255};
    static constexpr SDL_Color RoyalBlue = {65, 105, 225, 255};
    static constexpr SDL_Color Transparent = {0, 0, 0, 0};

    // Modern UI surface palette — designed to look good out of the box.
    // Use these for app backgrounds, panels, and component defaults.
    static constexpr SDL_Color BgDeep = {14, 17, 22, 255}; // App background
    static constexpr SDL_Color BgSurface = {22, 27, 34, 255}; // Panels
    static constexpr SDL_Color BgElevated = {30, 37, 46, 255}; // Buttons / inputs idle
    static constexpr SDL_Color BgHover = {42, 51, 62, 255}; // Hover state
    static constexpr SDL_Color BgActive = {52, 63, 76, 255}; // Active/pressed state
    static constexpr SDL_Color Border = {58, 68, 82, 255}; // Subtle border
    static constexpr SDL_Color BorderStrong = {92, 108, 128, 255}; // Focus / emphasis border

    static constexpr SDL_Color TextPrimary = {232, 237, 244, 255}; // Main text
    static constexpr SDL_Color TextMuted = {148, 161, 178, 255}; // Secondary text / placeholder
    static constexpr SDL_Color TextSubtle = {92, 102, 118, 255}; // Disabled text

    static constexpr SDL_Color Accent = {99, 140, 255, 255}; // Brand accent (calm blue)
    static constexpr SDL_Color AccentHover = {122, 158, 255, 255};
    static constexpr SDL_Color AccentActive = {78, 120, 235, 255};

    // ───── helpers ──────────────────────────────────────────────────────────
    static constexpr SDL_Color withAlpha(SDL_Color c, Uint8 a) {
        return {c.r, c.g, c.b, a};
    }

    static SDL_Color lighten(SDL_Color c, float amount) {
        float a = amount < 0.0f ? 0.0f : (amount > 1.0f ? 1.0f : amount);
        auto mix = [a](Uint8 v) {
            return static_cast<Uint8>(static_cast<float>(v) + (255.0f - static_cast<float>(v)) * a);
        };
        return {mix(c.r), mix(c.g), mix(c.b), c.a};
    }

    static SDL_Color darken(SDL_Color c, float amount) {
        float a = amount < 0.0f ? 0.0f : (amount > 1.0f ? 1.0f : amount);
        auto mix = [a](Uint8 v) {
            return static_cast<Uint8>(static_cast<float>(v) * (1.0f - a));
        };
        return {mix(c.r), mix(c.g), mix(c.b), c.a};
    }

    static SDL_Color mix(SDL_Color a, SDL_Color b, float t) {
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        auto lerp = [t](Uint8 x, Uint8 y) {
            return static_cast<Uint8>(static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t);
        };
        return {lerp(a.r, b.r), lerp(a.g, b.g), lerp(a.b, b.b), lerp(a.a, b.a)};
    }

    static SDL_Color multiply(SDL_Color color1, SDL_Color color2) {
        SDL_Color result;

        result.r = (Uint8)(((int)color1.r * color2.r) / 255);
        result.g = (Uint8)(((int)color1.g * color2.g) / 255);
        result.b = (Uint8)(((int)color1.b * color2.b) / 255);
        result.a = (Uint8)(((int)color1.a * color2.a) / 255);

        return result;
    }

    static SDL_FColor toFColor(SDL_Color color) {
        SDL_FColor result;
        result.r = color.r / 255;
        result.g = color.g / 255;
        result.b = color.b / 255;
        result.a = color.a / 255;
        return result;
    }

};

struct Rect {
    float x;
    float y;
    float w;
    float h;
    Rect(float x, float y, float w, float h) {
        this->x = x;
        this->y = y;
        this->w = w;
        this->h = h;
    }
    Rect() : x(0), y(0), w(0), h(0) {}
};

struct Circle {
    float2 pos;
    float radius;
};

class Sprite;

struct Triangle {
    float2 pointA;
    float2 pointB;
    float2 pointC;
};


struct Line {
    float2 startPos;
    float2 endPos;
    int thickness = 5;
};

struct NineSlice {
    const Sprite* sprite = nullptr;
    int top = 0;
    int bottom = 0;
    int left = 0;
    int right = 0;

    NineSlice() = default;
    NineSlice(const Sprite* s, int t, int b, int l, int r)
        : sprite(s), top(t), bottom(b), left(l), right(r) {}
    
    [[nodiscard]] bool isValid() const { return sprite != nullptr; }
};

struct Shape {
    std::vector<float2> points;
};

struct Button {
    Rect rect;
    bool isClicked = false;
    bool isClickedOnce = false;
    bool isHovered = false;

    // Sensible modern defaults, leave alone for a clean look, or override per-button.
    SDL_Color bgColor = Color::BgElevated;
    SDL_Color outlineColor = Color::Border;
    SDL_Color textColor = Color::TextPrimary;
    int outlineWidth = 1;
    std::string text;
    int textSize = 18;
    int roundRadius = 8;
    
    // Animation settings
    float animationSpeed = 10.0f;
    float clickScale = 0.97f; // Scale factor when pressed (1.0 = no scale)
    
    float hoverProgress = 0.0f; // 0 to 1, for animation transitions
    float clickProgress = 0.0f; // 0 to 1

    // State colors (auto-derived if Transparent)
    SDL_Color hoverColor = Color::Transparent;
    SDL_Color clickColor = Color::Transparent;
    SDL_Color hoverOutlineColor = Color::Transparent;
    SDL_Color clickOutlineColor = Color::Transparent;
    int hoverOutlineWidth = -1;
    int clickOutlineWidth = -1;

    void update(float dt) {
        if (isHovered) {
            hoverProgress = std::min(1.0f, hoverProgress + dt * animationSpeed);
        } else {
            hoverProgress = std::max(0.0f, hoverProgress - dt * animationSpeed);
        }

        if (isClicked) {
            clickProgress = std::min(1.0f, clickProgress + dt * animationSpeed * 2.0f);
        } else {
            clickProgress = std::max(0.0f, clickProgress - dt * animationSpeed);
        }
    }

    [[nodiscard]] SDL_Color getEffectiveBgColor() const {
        SDL_Color hCol = (hoverColor.a == 0) ? Color::lighten(bgColor, 0.1f) : hoverColor;
        SDL_Color cCol = (clickColor.a == 0) ? Color::darken(bgColor, 0.2f) : clickColor;
        SDL_Color mixed = Color::mix(bgColor, hCol, hoverProgress);
        return Color::mix(mixed, cCol, clickProgress);
    }

    [[nodiscard]] SDL_Color getEffectiveOutlineColor() const {
        SDL_Color hOut = (hoverOutlineColor.a == 0) ? outlineColor : hoverOutlineColor;
        SDL_Color cOut = (clickOutlineColor.a == 0) ? outlineColor : clickOutlineColor;
        SDL_Color mixed = Color::mix(outlineColor, hOut, hoverProgress);
        return Color::mix(mixed, cOut, clickProgress);
    }

    [[nodiscard]] int getEffectiveOutlineWidth() const {
        if (clickOutlineWidth != -1 && clickProgress > 0.5f) return clickOutlineWidth;
        if (hoverOutlineWidth != -1 && hoverProgress > 0.5f) return hoverOutlineWidth;
        return outlineWidth;
    }

    [[nodiscard]] Rect getAnimatedRect() const {
        if (clickProgress <= 0.0f || clickScale >= 1.0f) return rect;
        float currentScale = 1.0f - (clickProgress * (1.0f - clickScale));
        float dw = rect.w * (1.0f - currentScale);
        float dh = rect.h * (1.0f - currentScale);
        return { rect.x + dw / 2.0f, rect.y + dh / 2.0f, rect.w - dw, rect.h - dh };
    }
};

// The value will always range 0 - 1. Multiply on consumption to map to your domain.
struct Slider {
    Rect rect;
    float value = 0.5f;
    bool enabled = true;
    SDL_Color color = Color::BgElevated; // Track background
    SDL_Color fillColor = Color::Accent; // Fill (auto-used when set, derived otherwise)
    SDL_Color knobColor = Color::White;
    SDL_Color knobOutlineColor = Color::Border;
    int knobSize = 18;
    int roundRadius = 4;
    float step = 0.0f; // 0 means continuous

    // State
    bool isDragging = false;
    float hoverProgress = 0.0f;
    float animationSpeed = 10.0f;

    void update(float dt, bool isHovered) {
        if (enabled && (isHovered || isDragging)) {
            hoverProgress = std::min(1.0f, hoverProgress + dt * animationSpeed);
        } else {
            hoverProgress = std::max(0.0f, hoverProgress - dt * animationSpeed);
        }
    }
};

struct InputField {
    Rect rect;
    std::string value;
    std::string placeholder; // Shown when value is empty
    bool enabled = false; // Active editing state (focus); managed by the renderer
    bool hasStartedTextInput = false; // Internal flag to ensure SDL_StartTextInput is called once
    SDL_Color color = Color::BgElevated;
    SDL_Color textColor = Color::TextPrimary;
    SDL_Color placeholderColor = Color::TextMuted;
    SDL_Color borderColor = Color::Border;
    SDL_Color focusColor = Color::Accent;
    SDL_Color selectionColor = { 0, 120, 215, 128 }; // Default blueish highlight
    int textSize = 18;
    int roundRadius = 8;
    float animationSpeed = 10.0f;
    float focusProgress = 0.0f;
    bool multiLine = false;
    bool wrap = false;

    // Advanced state
    int cursorPos = 0;
    int selectionStart = -1; // -1 means no selection
    float cursorTimer = 0.0f;
    float backspaceTimer = 0.0f;
    float scrollOffset = 0.0f;
    float verticalScrollOffset = 0.0f;
    bool cursorVisible = true;

    // Filtering
    bool numericOnly = false;
    std::string allowedChars;

    // Terminal/Protected features
    int protectedLen = 0;
    std::function<void(const std::string&)> onTextSubmit;
    std::function<void(class Renderer&, const std::string&, float2, SDL_Color, int)> lineRenderer;

    void update(float dt) {
        if (enabled) {
            focusProgress = std::min(1.0f, focusProgress + dt * animationSpeed);
            cursorTimer += dt;
            if (cursorTimer >= 0.5f) {
                cursorTimer -= 0.5f;
                cursorVisible = !cursorVisible;
            }
        } else {
            focusProgress = std::max(0.0f, focusProgress - dt * animationSpeed);
            cursorVisible = false;
            cursorTimer = 0.0f;
            selectionStart = -1;
        }

        // Clamp cursorPos
        if (cursorPos < 0) cursorPos = 0;
        if (cursorPos > (int)value.length()) cursorPos = (int)value.length();
    }

    void clearSelection() { selectionStart = -1; }
    [[nodiscard]] bool hasSelection() const { return selectionStart != -1 && selectionStart != cursorPos; }
    [[nodiscard]] std::pair<int, int> getSelectionRange() const {
        if (!hasSelection()) return {cursorPos, cursorPos};
        return {std::min(selectionStart, cursorPos), std::max(selectionStart, cursorPos)};
    }

    void appendText(const std::string& text) {
        value += text;
        cursorPos = (int)value.length();
        protectedLen = (int)value.length();
    }

    [[nodiscard]] SDL_Color getEffectiveBorderColor() const {
        return Color::mix(borderColor, focusColor, focusProgress);
    }
};

struct ColorPicker {
    Rect rect;
    SDL_Color color = Color::White;
    float hue = 0.0f;
    float saturation = 1.0f;
    float value = 1.0f;
    bool enabled = true;

    // State
    bool isDraggingHue = false;
    bool isDraggingSV = false;

    void updateFromColor() {
        float r = color.r / 255.0f;
        float g = color.g / 255.0f;
        float b = color.b / 255.0f;
        float maxVal = std::max({r, g, b});
        float minVal = std::min({r, g, b});
        float delta = maxVal - minVal;

        value = maxVal;
        saturation = (maxVal == 0) ? 0 : delta / maxVal;

        if (delta == 0) {
            hue = 0;
        } else {
            if (maxVal == r) {
                hue = 60.0f * fmodf(((g - b) / delta), 6.0f);
            } else if (maxVal == g) {
                hue = 60.0f * (((b - r) / delta) + 2.0f);
            } else if (maxVal == b) {
                hue = 60.0f * (((r - g) / delta) + 4.0f);
            }
            if (hue < 0) hue += 360.0f;
        }
    }

    void updateColorFromHSV() {
        float c = value * saturation;
        float x = c * (1.0f - fabsf(fmodf(hue / 60.0f, 2.0f) - 1.0f));
        float m = value - c;
        float r, g, b;
        if (hue >= 0 && hue < 60) { r = c; g = x; b = 0; }
        else if (hue >= 60 && hue < 120) { r = x; g = c; b = 0; }
        else if (hue >= 120 && hue < 180) { r = 0; g = c; b = x; }
        else if (hue >= 180 && hue < 240) { r = 0; g = x; b = c; }
        else if (hue >= 240 && hue < 300) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }
        color.r = static_cast<Uint8>((r + m) * 255);
        color.g = static_cast<Uint8>((g + m) * 255);
        color.b = static_cast<Uint8>((b + m) * 255);
        color.a = 255;
    }
};

enum class NoiseType { NONE, WHITE, FRACTAL, WORLEY };

struct RawImage {
    int w;
    int h;
    std::vector<SDL_Color> data;
    NoiseType type = NoiseType::NONE;
    float scale = 0.0f;
    int octaves = 0;
    float persistence = 0.0f;

    [[nodiscard]] bool isValid() const {
        return w > 0 && h > 0;
    }

    void setPixel(int x, int y, SDL_Color color) {
        if (x >= 0 && x < w && y >= 0 && y < h) {
            data[y * w + x] = color;
        }
    }
};

struct Tail {
    std::vector<float2> points;
    int maxLength = 30;
};

inline float2 getTextSize(TTF_Font* font, const std::string& text, const int fontSize) {
    if (font == nullptr || text.empty()) {
        return {0.0f, 0.0f};
    }

    if (TTF_GetFontSize(font) != static_cast<float>(fontSize)) {
        TTF_SetFontSize(font, static_cast<float>(fontSize));
    }

    int w = 0;
    int h = 0;
    TTF_GetStringSize(font, text.c_str(), 0, &w, &h);

    return {static_cast<float>(w), static_cast<float>(h)};
}

struct Texture { SDL_Texture* handle = nullptr; int w = 0; int h = 0; bool isValid() const { return handle != nullptr && w > 0 && h > 0; } };

struct OutlineProperties {
    bool enabled = false;
    SDL_Color color{0, 0, 0, 255};
    int width{5};
    float opacity{1.0};
};

#endif //TRANSLUCENCEWORKSPACE_CORE_HPP
