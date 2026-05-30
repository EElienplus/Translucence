//
// Created by Stěpán Toman on 07.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_MATH_HPP
#define TRANSLUCENCEWORKSPACE_MATH_HPP

#include "T_Core.hpp"
#include <Application.hpp>
#include <random>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace Math {
    inline float pi = 3.1415927f;
    inline float tau = pi*2;

    inline const std::string BASE_CHARS =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "!#$%&()*+,-./:;<=>?@[]^_`{|}~";

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

    inline float2 subtract(float2 a, float2 b) {
        return a - b;
    }

    inline float2 subtract(float2 a, float b) {
        return {a.x - b, a.y - b};
    }

    inline float2 add(float2 a, float2 b) {
        return a + b;
    }

    inline float2 add(float b, float2 a) { // Fixed ordering helper signature swap flexibility
        return {a.x + b, a.y + b};
    }

    inline float2 add(float2 a, float b) {
        return {a.x + b, a.y + b};
    }

    inline float2 multiply(float2 a, float b) {
        return a * b;
    }

    inline float2 divide(float2 a, float b) {
        if (b == 0.0f) return {0, 0};
        return a / b;
    }

    inline float length(float2 a) {
        return std::sqrt(a.x * a.x + a.y * a.y);
    }

    inline float distance(float2 a, float2 b) {
        return length(subtract(a, b));
    }

    inline float2 normalize(float2 a) {
        float l = length(a);
        if (l == 0.0f) return {0, 0};
        return divide(a, l);
    }

    inline float2 perpendicular(float2 a) {
        return {-a.y, a.x};
    }

    inline int charToValue(char c) {
        size_t pos = BASE_CHARS.find(c);
        if (pos == std::string::npos) {
            throw std::invalid_argument("Character not found in standard charset.");
        }
        return static_cast<int>(pos);
    }

    /**
     * @brief Core arbitrary-precision vector base conversion engine.
     * Implements base scaling linearly via dynamic long division/multiplication arrays.
     */
    inline std::vector<int> convertBase(const std::vector<int>& inputDigits, int fromBase, int toBase) {
        if (fromBase < 2 || toBase < 2) {
            throw std::out_of_range("Bases must be at least 2.");
        }

        std::vector<int> result;
        if (inputDigits.empty()) {
            result.push_back(0);
            return result;
        }

        // We process data iteratively using a base-conversion simulation
        // matching array structures of BigInt setups.
        std::vector<int> currentDigits = inputDigits;

        // Strip leading zeros to keep processing clean and fast
        size_t firstNonZero = 0;
        while (firstNonZero < currentDigits.size() && currentDigits[firstNonZero] == 0) {
            firstNonZero++;
        }
        if (firstNonZero == currentDigits.size()) {
            return {0};
        }

        while (firstNonZero < currentDigits.size()) {
            unsigned long long remainder = 0;

            for (size_t i = firstNonZero; i < currentDigits.size(); ++i) {
                if (currentDigits[i] >= fromBase || currentDigits[i] < 0) {
                    throw std::invalid_argument("Digit value out of bounds for source base.");
                }
                unsigned long long current = currentDigits[i] + remainder * fromBase;
                currentDigits[i] = static_cast<int>(current / toBase);
                remainder = current % toBase;
            }

            result.push_back(static_cast<int>(remainder));

            // Advance the tracking line past any newly generated leading zeros
            while (firstNonZero < currentDigits.size() && currentDigits[firstNonZero] == 0) {
                firstNonZero++;
            }
        }

        std::reverse(result.begin(), result.end());
        return result;
    }

    /**
     * @brief Converts an infinitely large string representation of a number between bases 2-94.
     */
    inline std::string convertBase(std::string valueStr, int fromBase, int toBase) {
        int maxBase = static_cast<int>(BASE_CHARS.size());
        if (fromBase < 2 || fromBase > maxBase || toBase < 2 || toBase > maxBase) {
            throw std::out_of_range("Bases must be between 2 and " + std::to_string(maxBase));
        }

        if (fromBase == 16 && (valueStr.starts_with("0x") || valueStr.starts_with("0X"))) {
            valueStr.erase(0, 2);
        } else if (fromBase == 2 && (valueStr.starts_with("0b") || valueStr.starts_with("0B"))) {
            valueStr.erase(0, 2);
        }

        if (valueStr.empty()) return "0";

        // Map character strings straight to token values
        std::vector<int> rawDigits;
        rawDigits.reserve(valueStr.size());
        for (char c : valueStr) {
            rawDigits.push_back(charToValue(c));
        }

        // Process through the arbitrary precision engine
        std::vector<int> targetDigits = convertBase(rawDigits, fromBase, toBase);

        std::string resultStr = "";
        for (int d : targetDigits) {
            resultStr += BASE_CHARS[d];
        }
        return resultStr;
    }

    /**
     * @brief Converts a native 64-bit unsigned integer directly into an infinitely large string representation.
     */
    inline std::string convertBase(unsigned long long value, int toBase) {
        int maxBase = static_cast<int>(BASE_CHARS.size());
        if (toBase < 2 || toBase > maxBase) {
            throw std::out_of_range("Base must be between 2 and " + std::to_string(maxBase));
        }
        if (value == 0) return "0";

        std::string result = "";
        while (value > 0) {
            result += BASE_CHARS[value % toBase];
            value /= toBase;
        }

        std::reverse(result.begin(), result.end());
        return result;
    }

    /**
     * @brief Converts a base string representation back down to a native 64-bit integer.
     * Note: This safely catches overflows if trying to store an oversized string inside a native primitive.
     */
    inline unsigned long long convertBase(std::string valueStr, int fromBase) {
        int maxBase = static_cast<int>(BASE_CHARS.size());
        if (fromBase < 2 || fromBase > maxBase) {
            throw std::out_of_range("Base must be between 2 and " + std::to_string(maxBase));
        }

        if (fromBase == 16 && (valueStr.starts_with("0x") || valueStr.starts_with("0X"))) {
            valueStr.erase(0, 2);
        } else if (fromBase == 2 && (valueStr.starts_with("0b") || valueStr.starts_with("0B"))) {
            valueStr.erase(0, 2);
        }

        if (valueStr.empty()) return 0;

        unsigned long long result = 0;
        unsigned long long power = 1;

        for (int i = static_cast<int>(valueStr.size()) - 1; i >= 0; i--) {
            int digitValue = charToValue(valueStr[i]);
            if (digitValue >= fromBase) {
                throw std::invalid_argument("Digit exceeds base limit.");
            }

            result += digitValue * power;

            if (i > 0 && result > (std::numeric_limits<unsigned long long>::max() / fromBase)) {
                throw std::overflow_error("Value is too massive to fit inside a single standard 64-bit primitive integer.");
            }
            power *= fromBase;
        }

        return result;
    }
}

#endif //TRANSLUCENCEWORKSPACE_MATH_HPP