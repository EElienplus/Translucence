#include "ImageProcess.hpp"
#include <cmath>
#include <algorithm>

float ImageProcess::valueNoise2D(float x, float y) {
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