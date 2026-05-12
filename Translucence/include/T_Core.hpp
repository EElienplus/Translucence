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
#include <cmath>
#include <vector>

namespace fs = std::filesystem;

template <typename T, typename T1>
T cast(T1 value) {
    return static_cast<T>(value);
}

template <typename T>
std::string toString(T value) {
    return std::to_string(value);
}

template <typename T>
void print(T value) {
    std::cout << value;
}

template <typename T>
void println(T value) {
    std::cout << value << "\n";
}

struct float2 {
    float x, y;
};

struct float3 {
    float x, y, z;
};

struct float4 {
    float x, y, z, w;
};

struct Color {
    // Classic
    static constexpr SDL_Color White       = { 255, 255, 255, 255 };
    static constexpr SDL_Color Black       = { 0,   0,   0,   255 };
    static constexpr SDL_Color Red         = { 255, 0,   0,   255 };
    static constexpr SDL_Color Green       = { 0,   255, 0,   255 };
    static constexpr SDL_Color Blue        = { 0,   0,   255, 255 };
    static constexpr SDL_Color Cyan        = { 0,   255, 255, 255 };
    static constexpr SDL_Color Magenta     = { 255, 0,   255, 255 };
    static constexpr SDL_Color Yellow      = { 255, 255, 0,   255 };

    // Earthy / saturated
    static constexpr SDL_Color NavyBlue    = { 32,  42,  68,  255 };
    static constexpr SDL_Color Crimson     = { 220, 20,  60,  255 };
    static constexpr SDL_Color Maroon      = { 128, 0,   0,   255 };
    static constexpr SDL_Color Indigo      = { 75,  0,   130, 255 };
    static constexpr SDL_Color Charcoal    = { 54,  69,  79,  255 };
    static constexpr SDL_Color SlateGray   = { 112, 128, 144, 255 };
    static constexpr SDL_Color DimGray     = { 105, 105, 105, 255 };
    static constexpr SDL_Color DarkGray    = { 65,  65,  65,  255 };
    static constexpr SDL_Color ForestGreen = { 34,  139, 34,  255 };
    static constexpr SDL_Color Teal        = { 0,   128, 128, 255 };
    static constexpr SDL_Color Orange      = { 255, 165, 0,   255 };
    static constexpr SDL_Color DeepPink    = { 255, 20,  147, 255 };
    static constexpr SDL_Color Gold        = { 255, 215, 0,   255 };
    static constexpr SDL_Color SkyBlue     = { 135, 206, 235, 255 };
    static constexpr SDL_Color SpringGreen = { 0,   255, 127, 255 };
    static constexpr SDL_Color Violet      = { 238, 130, 238, 255 };
    static constexpr SDL_Color Chocolate   = { 210, 105, 30,  255 };
    static constexpr SDL_Color Sienna      = { 160, 82,  45,  255 };
    static constexpr SDL_Color SaddleBrown = { 139, 69,  19,  255 };
    static constexpr SDL_Color Goldenrod   = { 218, 165, 32,  255 };
    static constexpr SDL_Color Olive       = { 128, 128, 0,   255 };
    static constexpr SDL_Color Mint        = { 189, 252, 201, 255 };
    static constexpr SDL_Color Peach       = { 255, 218, 185, 255 };
    static constexpr SDL_Color Lavender    = { 230, 230, 250, 255 };
    static constexpr SDL_Color Ivory       = { 255, 255, 240, 255 };
    static constexpr SDL_Color Silver      = { 192, 192, 192, 255 };
    static constexpr SDL_Color Transparent = { 0,   0,   0,   0   };

    // Modern UI surface palette — designed to look good out of the box.
    // Use these for app backgrounds, panels, and component defaults.
    static constexpr SDL_Color BgDeep      = { 14,  17,  22,  255 }; // App background
    static constexpr SDL_Color BgSurface   = { 22,  27,  34,  255 }; // Panels
    static constexpr SDL_Color BgElevated  = { 30,  37,  46,  255 }; // Buttons / inputs idle
    static constexpr SDL_Color BgHover     = { 42,  51,  62,  255 }; // Hover state
    static constexpr SDL_Color BgActive    = { 52,  63,  76,  255 }; // Active/pressed state
    static constexpr SDL_Color Border      = { 58,  68,  82,  255 }; // Subtle border
    static constexpr SDL_Color BorderStrong= { 92,  108, 128, 255 }; // Focus / emphasis border

    static constexpr SDL_Color TextPrimary = { 232, 237, 244, 255 }; // Main text
    static constexpr SDL_Color TextMuted   = { 148, 161, 178, 255 }; // Secondary text / placeholder
    static constexpr SDL_Color TextSubtle  = { 92,  102, 118, 255 }; // Disabled text

    static constexpr SDL_Color Accent      = { 99,  140, 255, 255 }; // Brand accent (calm blue)
    static constexpr SDL_Color AccentHover = { 122, 158, 255, 255 };
    static constexpr SDL_Color AccentActive= { 78,  120, 235, 255 };

    // ───── helpers ──────────────────────────────────────────────────────────
    static constexpr SDL_Color withAlpha(SDL_Color c, Uint8 a) {
        return { c.r, c.g, c.b, a };
    }

    static SDL_Color lighten(SDL_Color c, float amount) {
        float a = amount < 0.0f ? 0.0f : (amount > 1.0f ? 1.0f : amount);
        auto mix = [a](Uint8 v) {
            return static_cast<Uint8>(static_cast<float>(v) + (255.0f - static_cast<float>(v)) * a);
        };
        return { mix(c.r), mix(c.g), mix(c.b), c.a };
    }

    static SDL_Color darken(SDL_Color c, float amount) {
        float a = amount < 0.0f ? 0.0f : (amount > 1.0f ? 1.0f : amount);
        auto mix = [a](Uint8 v) {
            return static_cast<Uint8>(static_cast<float>(v) * (1.0f - a));
        };
        return { mix(c.r), mix(c.g), mix(c.b), c.a };
    }

    static SDL_Color mix(SDL_Color a, SDL_Color b, float t) {
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        auto lerp = [t](Uint8 x, Uint8 y) {
            return static_cast<Uint8>(static_cast<float>(x) + (static_cast<float>(y) - static_cast<float>(x)) * t);
        };
        return { lerp(a.r, b.r), lerp(a.g, b.g), lerp(a.b, b.b), lerp(a.a, b.a) };
    }
};

struct Rect {
    float x;
    float y;
    float w;
    float h;
};

struct Circle {
    float2 pos;
    float radius;
};

struct Triangle {
    float2 pointA;
    float2 pointB;
    float2 pointC;
};

struct Shape {
    std::vector<float2> points;
};

struct Button {
    Rect rect;
    bool isClicked = false;
    bool isClickedOnce = false;
    bool isHovered = false;

    // Sensible modern defaults — leave alone for a clean look, or override per-button.
    SDL_Color bgColor      = Color::BgElevated;
    SDL_Color outlineColor = Color::Border;
    SDL_Color textColor    = Color::TextPrimary;
    int outlineWidth = 1;
    std::string text;
    int textSize = 18;
    int roundRadius = 8;

    // When left as Transparent, the renderer derives hover/click colors automatically
    // from bgColor + outlineColor (so users only need to set one base color).
    SDL_Color hoverColor        = Color::Transparent;
    SDL_Color clickColor        = Color::Transparent;
    SDL_Color hoverOutlineColor = Color::Transparent;
    SDL_Color clickOutlineColor = Color::Transparent;
    int hoverOutlineWidth = -1;
    int clickOutlineWidth = -1;
};

// The value will always range 0 - 1. Multiply on consumption to map to your domain.
struct Slider {
    Rect rect;
    float value = 0.5f;
    SDL_Color color     = Color::BgElevated; // Track background
    SDL_Color fillColor = Color::Accent;     // Fill (auto-used when set, derived otherwise)
    SDL_Color knobColor = Color::White;
    int knobSize = 18;
};

struct InputField {
    Rect rect;
    std::string value;
    std::string placeholder; // Shown when value is empty
    bool enabled = false; // Active editing state (focus); managed by the renderer
    SDL_Color color       = Color::BgElevated;
    SDL_Color textColor   = Color::TextPrimary;
    SDL_Color placeholderColor = Color::TextMuted;
    SDL_Color borderColor = Color::Border;
    SDL_Color focusColor  = Color::Accent;
    int textSize = 18;
    int roundRadius = 8;
};

struct RawImage {
    int w;
    int h;
    std::vector<SDL_Color> data;
};

struct Tail {
    std::vector<float2> points;
    int maxLength = 30;
};

#endif //TRANSLUCENCEWORKSPACE_CORE_HPP
