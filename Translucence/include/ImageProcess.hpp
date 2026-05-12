//
// Created by Stěpán Toman on 07.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_IMAGEPROCESS_HPP
#define TRANSLUCENCEWORKSPACE_IMAGEPROCESS_HPP

#include "T_Core.hpp"
#include "Math.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

class ImageProcess {

public:
    static RawImage booleanNoise(int w, int h) {
        RawImage image;
        image.w = w;
        image.h = h;
        image.data.resize(w * h);

        for (int i = 0; i < image.data.size(); i++) {
            image.data[i] = (Math::randFloat() < 0.49f) ? Color::Black : Color::White;
        }
        return image;
    }

    static RawImage whiteNosie(int w, int h) {
        RawImage image;
        image.w = w;
        image.h = h;
        image.data.resize(w * h);

        for (int i = 0; i < image.data.size(); i++) {
            Uint8 val = cast<Uint8>(Math::randInt(0, 255));
            image.data[i] = SDL_Color{val, val, val, 255};
        }

        return image;
    }

    // Foundation: 2D Value Noise
    static float valueNoise2D(float x, float y) {
        int ix = cast<int>(std::floor(x));
        int iy = cast<int>(std::floor(y));
        float fx = x - cast<float>(ix);
        float fy = y - cast<float>(iy);

        auto lerp = [](float a, float b, float t) { return a + t * (b - a); };
        auto fade = [](float t) { return t * t * (3.0f - 2.0f * t); };

        float u = fade(fx);
        float v = fade(fy);

        auto hash = [](int x, int y) {
            unsigned int h = x * 374761393 + y * 668265263;
            h = (h ^ (h >> 13)) * 1274126177;
            return cast<float>(h & 0x7fffffff) / 2147483647.0f;
        };

        float n00 = hash(ix, iy);
        float n10 = hash(ix + 1, iy);
        float n01 = hash(ix, iy + 1);
        float n11 = hash(ix + 1, iy + 1);

        return lerp(lerp(n00, n10, u), lerp(n01, n11, u), v);
    }

    // Fractal Noise with Scale:
    // Higher scale = larger, smoother features.
    static RawImage fractalNoise(int w, int h, float scale = 50.0f, int octaves = 6, float persistence = 0.5f) {
        RawImage image;
        image.w = w;
        image.h = h;
        image.data.resize(w * h);

        // Avoid division by zero
        if (scale <= 0.1f) scale = 0.1f;

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                float amplitude = 1.0f;
                float freq = 1.0f / scale;
                float noiseValue = 0.0f;
                float maxValue = 0.0f;

                for (int i = 0; i < octaves; i++) {
                    noiseValue += valueNoise2D(x * freq, y * freq) * amplitude;
                    maxValue += amplitude;
                    amplitude *= persistence;
                    freq *= 2.0f; // Each octave doubles the detail
                }

                Uint8 colorVal = cast<Uint8>((noiseValue / maxValue) * 255.0f);
                image.data[y * w + x] = SDL_Color{colorVal, colorVal, colorVal, 255};
            }
        }
        return image;
    }

    // Worley Noise with Scale:
    // Scale here determines the average distance between cell centers.
    static RawImage worleyNoise(int w, int h, float scale = 40.0f) {
        RawImage image;
        image.w = w;
        image.h = h;
        image.data.resize(w * h);

        // Calculate number of points based on area and scale
        int numPoints = cast<int>((w * h) / (scale * scale));
        if (numPoints < 2) numPoints = 2;

        struct Point { float x, y; };
        std::vector<Point> points(numPoints);
        for (int i = 0; i < numPoints; i++) {
            points[i] = { Math::randFloatRange(0, cast<float>(w)), Math::randFloatRange(0, cast<float>(h)) };
        }

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                float minDist = 1e10f;
                for (const auto& p : points) {
                    float dx = p.x - x;
                    float dy = p.y - y;
                    float distSq = dx*dx + dy*dy; // Use squared distance for speed
                    if (distSq < minDist) minDist = distSq;
                }

                float dist = std::sqrt(minDist);
                // Normalize based on scale to keep contrast consistent
                float normalized = std::min(dist / scale, 1.0f);
                Uint8 val = cast<Uint8>(normalized * 255.0f);
                image.data[y * w + x] = SDL_Color{val, val, val, 255};
            }
        }
        return image;
    }
};

#endif //TRANSLUCENCEWORKSPACE_IMAGEPROCESS_HPP