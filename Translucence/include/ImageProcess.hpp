//
// Created by Štěpán Toman on 07.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_IMAGEPROCESS_HPP
#define TRANSLUCENCEWORKSPACE_IMAGEPROCESS_HPP

#include "T_Core.hpp"
class ImageProcess {
public:
    static RawImage booleanNoise(int w, int h) {
        RawImage img; img.w = w; img.h = h; img.type = NoiseType::WHITE; // Boolean logic handled via White + threshold in renderer
        return img;
    }
    static RawImage whiteNoise(int w, int h) {
        RawImage img; img.w = w; img.h = h; img.type = NoiseType::WHITE;
        return img;
    }
    static float valueNoise2D(float x, float y);
    static RawImage fractalNoise(int w, int h, float scale = 50.0f, int octaves = 6, float persistence = 0.5f) {
        RawImage img; img.w = w; img.h = h; img.type = NoiseType::FRACTAL;
        img.scale = scale; img.octaves = octaves; img.persistence = persistence;
        return img;
    }
    static RawImage worleyNoise(int w, int h, float scale = 40.0f) {
        RawImage img; img.w = w; img.h = h; img.type = NoiseType::WORLEY;
        img.scale = scale;
        return img;
    }

    static void fillImage(RawImage& image, SDL_Color color);
    static void smoothenImage(RawImage& image, int radius);

};

#endif //TRANSLUCENCEWORKSPACE_IMAGEPROCESS_HPP
