//
// Created by Stěpán Toman on 10.05.2026.
//

#include "../include/Sprite.hpp"
#include <algorithm>

std::vector<Sprite*> Sprite::sprites;

Sprite::Sprite(Application& application, const std::string& argFilePath) : app(application) {
    sprites.push_back(this);
    filePath = argFilePath;
    surface = IMG_Load(filePath.c_str());
    if (!surface) {
        SDL_Log("Failed to load surface from %s: %s", filePath.c_str(), SDL_GetError());
        width = 0;
        height = 0;
        texture = nullptr;
        return;
    }

    texture = SDL_CreateTextureFromSurface(app.getRenderer(), surface);
    if (!texture) {
        SDL_Log("Failed to create texture from %s: %s", filePath.c_str(), SDL_GetError());
        width = 0;
        height = 0;
        return;
    }

    if (!SDL_GetTextureSize(texture, &width, &height)) {
        SDL_Log("Failed to get texture size for %s: %s", filePath.c_str(), SDL_GetError());
    }

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
}

Sprite::~Sprite() {
    sprites.erase(std::remove(sprites.begin(), sprites.end(), this), sprites.end());
    SDL_DestroySurface(surface);
    SDL_DestroyTexture(texture);
}


SDL_Surface* Sprite::getSurface() const {
    return surface;
}

SDL_Texture* Sprite::getTexture() const {
    return texture;
}

float2 Sprite::getCenterPos() {
    float x, y;
    x = width/2;
    y = height/2;
    return {x, y};
}
float2 Sprite::getFeetPos() {
    float x, y;
    x = width/2;
    y = height;
    return {x, y};
}
void Sprite::update(float scale) {
    this->scale = scale;
    collider.spriteToCollider(*this, scale);
}

void Sprite::updatePhysics(float dt) {
    if (!autoUpdatePhysics) return;

    if (useGravity) {
        applyGravity(gravityStrength, dt);
    } else {
        pos.x += velocity.x * dt;
        pos.y += velocity.y * dt;
    }

    for (auto* otherCollider : Collider::getColliders()) {
        if (otherCollider == &collider) continue;
        resolveCollision(*otherCollider, scale);
    }
}

void Sprite::assignCollider(float scale) {
    collider.spriteToCollider(*this, scale);
}

void Sprite::applyGravity(float strength, float dt) {
    velocity.y += strength * dt;
    pos.y += velocity.y * dt;
}

void Sprite::resolveCollision(const Collider& other, float scale) {
    update(scale);
    if (collider.checkCollision(other)) {
        if (other.getType() == ColliderType::Rect) {
            Rect r = other.getRect();
            // Simple snapping for ground (top of the other rect)
            if (velocity.y > 0) {
                pos.y = r.y - height * scale;
                velocity.y = 0;
                grounded = true;
                update(scale);
            }
        }
    } else {
        grounded = false;
    }
}


void Sprite::wadMovement(float speed, float jumpStrength, float dt) {
    if (Input::isKeyDown(Input::Key::A)) pos.x -= speed * dt;
    if (Input::isKeyDown(Input::Key::D)) pos.x += speed * dt;
    if (Input::isKeyDown(Input::Key::W)) {
        if (grounded) {
            velocity.y = -jumpStrength;
            grounded = false;
        }
    }
}


