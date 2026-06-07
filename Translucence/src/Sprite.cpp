//
// Created by Stěpán Toman on 10.05.2026.
//

#include <Sprite.hpp>
#include <algorithm>

#include "Input.hpp"
#include "Math.hpp"

void Sprite::mirrorVertical() const {
   SDL_FlipSurface(surface, SDL_FLIP_VERTICAL);
}
void Sprite::mirrorHorizontal() const {
    SDL_FlipSurface(surface, SDL_FLIP_HORIZONTAL);
}
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
void Sprite::update(float argScale, float dt) {
    this->scale = argScale;
    collider.spriteToCollider(*this, argScale);
    updatePhysics(dt);
}

void Sprite::updatePhysics(float dt) {
    if (!autoUpdatePhysics) return;

    if (useGravity) {
        velocity.y += gravityStrength * dt;
    }

    // Horizontal pass: move and resolve
    pos.x += velocity.x * dt;
    collider.spriteToCollider(*this, scale);
    for (auto* otherCollider : Collider::getColliders()) {
        if (otherCollider == &collider) continue;
        resolveCollision(*otherCollider, scale, true); // true for horizontal
    }

    // Vertical pass: move and resolve
    pos.y += velocity.y * dt;
    collider.spriteToCollider(*this, scale);
    grounded = false;
    for (auto* otherCollider : Collider::getColliders()) {
        if (otherCollider == &collider) continue;
        resolveCollision(*otherCollider, scale, false); // false for vertical
    }
}

void Sprite::assignCollider(float argScale) {
    collider.spriteToCollider(*this, argScale);
}

void Sprite::applyGravity(float strength, float dt) {
    velocity.y += strength * dt;
    pos.y += velocity.y * dt;
}

void Sprite::resolveCollision(const Collider& other, float argScale, bool horizontalOnly) {
    if (collider.checkCollision(other)) {
        if (other.getType() == ColliderType::Rect) {
            Rect r = other.getRect();
            Rect p = collider.getRect();

            if (horizontalOnly) {
                float overlapLeft   = (p.x + p.w) - r.x;
                float overlapRight  = (r.x + r.w) - p.x;
                if (overlapLeft < overlapRight) {
                    pos.x = r.x - width * argScale;
                } else {
                    pos.x = r.x + r.w;
                }
                velocity.x = 0;
            } else {
                float overlapTop    = (p.y + p.h) - r.y;
                float overlapBottom = (r.y + r.h) - p.y;
                if (overlapTop < overlapBottom) {
                    pos.y = r.y - height * argScale;
                    velocity.y = 0;
                    grounded = true;
                } else {
                    pos.y = r.y + r.h;
                    if (velocity.y < 0) velocity.y = 0;
                }
            }
            // Sync collider position after resolution
            collider.spriteToCollider(*this, argScale);
        }
    }
}


void Sprite::wadMovement(float speed, float jumpStrength, float dt, float acceleration, float friction) {
    float moveInput = 0.0f;
    if (Input::isKeyDown(Input::Key::A)) moveInput -= 1.0f;
    if (Input::isKeyDown(Input::Key::D)) moveInput += 1.0f;

    if (moveInput != 0.0f) {
        float currentAcc = acceleration;
        // Snappy turning: if we are moving against our current velocity, accelerate faster
        if (velocity.x != 0 && (moveInput * velocity.x < 0)) {
            currentAcc += friction * 2.0f;
        }
        velocity.x += moveInput * currentAcc * dt;
    } else {
        float frictionAmount = friction * 2.5f * dt;
        if (std::abs(velocity.x) < frictionAmount) {
            velocity.x = 0;
        } else {
            velocity.x -= (velocity.x > 0 ? 1.0f : -1.0f) * frictionAmount;
        }
    }

    // Clamp horizontal velocity
    if (std::abs(velocity.x) > speed) {
        velocity.x = (velocity.x > 0 ? 1.0f : -1.0f) * speed;
    }

    if (Input::isKeyDown(Input::Key::W)) {
        if (grounded) {
            velocity.y = -jumpStrength;
            grounded = false;
        }
    }
}

void Sprite::updateDragDrop() {
    float2 mousePos = Input::getMousePos();

    if (Input::isMouseClicked() && !isDragging && Input::isMouseHoveringRect(Input::getMousePos(), {pos.x, pos.y, width, height})) {
        isDragging = true;
        mouseOffset = Math::subtract(pos, mousePos);
    }

    if (!Input::isMouseClicked()) {
        isDragging = false;
    }

    if (isDragging) {
        pos = Math::add(mousePos, mouseOffset);
    }
}
