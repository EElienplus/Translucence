//
// Created by Stěpán Toman on 07.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_MATH_HPP
#define TRANSLUCENCEWORKSPACE_MATH_HPP

#include "T_Core.hpp"
#include <random>

namespace Math {
    inline float pi = 3.1415927f;
    inline float tau = pi*2;

    inline int getSizeWithinButton(TTF_Font* font, std::string text, Rect button) {
        int fontSize = 32; // Starting "ideal" size
        const float padding = 0.8f; // Text should take up 80% of button width

        int w, h;
        TTF_SetFontSize(font, (float)fontSize);

        // Check dimensions and shrink if necessary
        while (fontSize > 8) {
            if (TTF_GetStringSize(font, text.c_str(), 0, &w, &h)) {
                if (w <= button.w * padding) break;
            }
            fontSize -= 2;
            TTF_SetFontSize(font, (float)fontSize);
        }

        return fontSize;
    }

    inline float2 getPosWithinButton(TTF_Font* font, std::string text, int fontSize, Rect button) {
        int w, h;
        TTF_SetFontSize(font, (float)fontSize);

        if (TTF_GetStringSize(font, text.c_str(), 0, &w, &h)) {
            float2 pos;
            pos.x = button.x + (button.w - (float)w) / 2.0f;
            pos.y = button.y + (button.h - (float)h) / 2.0f;
            return pos;
        }

        return { button.x, button.y }; // Fallback if something gets fucked up
    }

    inline bool colorMatch(SDL_Color color1, SDL_Color color2) {
        return ((color1.r == color2.r) && (color1.g == color2.g) && (color1.b == color2.b) && (color1.a == color2.a));
    }


    // A helper to keep the generator persistent and efficient
    inline std::mt19937& getRNG() {
        static std::random_device dev;
        static std::mt19937 rng(dev());
        return rng;
    }

    inline int randInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(getRNG());
    }

    inline float randFloatRange(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(getRNG());
    }

    inline float randFloat() {
        // Standard 0.0 to 1.0 range
        return randFloatRange(0.0f, 1.0f);
    }

    inline float2 lerpFloat2(float2 a, float2 b, float t) {
        return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
    }

    inline float clamp(float min, float max, float value) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    inline float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    inline bool isOutsideScreen(const Rect& rect, const Application& app) {
        return rect.x < 0 || rect.y < 0 || rect.x + rect.w > (float)app.getWidth() || rect.y + rect.h > (float)app.getHeight();
    }

    inline bool isOutsideScreen(const Circle& circle, const Application& app) {
        return circle.pos.x - circle.radius < 0 || circle.pos.y - circle.radius < 0 || 
               circle.pos.x + circle.radius > (float)app.getWidth() || circle.pos.y + circle.radius > (float)app.getHeight();
    }

    inline bool isOutsideScreen(const Triangle& triangle, const Application& app) {
        auto isPointOutside = [&](float2 p) {
            return p.x < 0 || p.y < 0 || p.x > (float)app.getWidth() || p.y > (float)app.getHeight();
        };
        return isPointOutside(triangle.pointA) || isPointOutside(triangle.pointB) || isPointOutside(triangle.pointC);
    }

    inline bool isOutsideScreen(const Shape& shape, const Application& app) {
        for (const auto& p : shape.points) {
            if (p.x < 0 || p.y < 0 || p.x > (float)app.getWidth() || p.y > (float)app.getHeight()) return true;
        }
        return false;
    }

}

#endif //TRANSLUCENCEWORKSPACE_MATH_HPP
