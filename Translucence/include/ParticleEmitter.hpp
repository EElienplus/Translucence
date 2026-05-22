//
// Created by Stěpán Toman on 21.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_PARTICLEEMMITER_HPP
#define TRANSLUCENCEWORKSPACE_PARTICLEEMMITER_HPP

#include <T_Core.hpp>
#include <vector>

struct Particle {
    float2 pos;
    float2 vel;
    float life;
    float maxLife;
    SDL_Color startColor;
    SDL_Color endColor;
    float startSize;
    float endSize;

    Particle(float2 pos, float2 vel, float maxLife, SDL_Color startColor, SDL_Color endColor, float startSize, float endSize)
        : pos(pos), vel(vel), life(maxLife), maxLife(maxLife), startColor(startColor), endColor(endColor), startSize(startSize), endSize(endSize) {}
};

class ParticleEmitter {
public:
    float2 pos{};
    float2 velocityMin{-1.0f, -1.0f};
    float2 velocityMax{1.0f, 1.0f};
    float minLifeTime{1.0f};
    float maxLifeTime{2.0f};
    SDL_Color startColor{255, 255, 255, 255};
    SDL_Color endColor{255, 255, 255, 0};
    float startSize{5.0f};
    float endSize{1.0f};
    float emissionRate{10.0f}; // particles per second

    ParticleEmitter() = default;
    ParticleEmitter(float2 pos, float maxLifeTime, SDL_Color startColor, SDL_Color endColor, float startSize, float endSize) {
        this->pos = pos;
        this->minLifeTime = maxLifeTime * 0.5f;
        this->maxLifeTime = maxLifeTime;
        this->startColor = startColor;
        this->endColor = endColor;
        this->startSize = startSize;
        this->endSize = endSize;
    }
    ~ParticleEmitter() = default;

    void update(float deltaTime);
    void emit(int count = 1);

    const std::vector<Particle>& getParticles() const { return particles; }

private:
    std::vector<Particle> particles;
    float emissionTimer{0.0f};
};
#endif //TRANSLUCENCEWORKSPACE_PARTICLEEMMITER_HPP
