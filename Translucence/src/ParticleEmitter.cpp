#include "ParticleEmitter.hpp"
#include "Math.hpp"

void ParticleEmitter::update(float deltaTime) {
    // Emission logic
    if (emissionRate > 0) {
        emissionTimer += deltaTime;
        float interval = 1.0f / emissionRate;
        while (emissionTimer >= interval) {
            emit(1);
            emissionTimer -= interval;
        }
    }

    // Update particles
    for (auto it = particles.begin(); it != particles.end();) {
        it->life -= deltaTime;
        if (it->life <= 0) {
            it = particles.erase(it);
        } else {
            it->pos.x += it->vel.x * deltaTime;
            it->pos.y += it->vel.y * deltaTime;
            ++it;
        }
    }
}

void ParticleEmitter::emit(int count) {
    for (int i = 0; i < count; ++i) {
        float2 vel = {
            Math::randFloatRange(velocityMin.x, velocityMax.x),
            Math::randFloatRange(velocityMin.y, velocityMax.y)
        };
        float life = Math::randFloatRange(minLifeTime, maxLifeTime);
        particles.emplace_back(pos, vel, life, startColor, endColor, startSize, endSize);
    }
}
