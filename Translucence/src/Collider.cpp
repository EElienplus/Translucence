//
// Created by Stěpán Toman on 10.05.2026.
//

#include "Collider.hpp"
#include "Sprite.hpp"
#include <algorithm>

std::vector<Collider*> Collider::colliders;

Collider::Collider() {
    colliders.push_back(this);
}

Collider::~Collider() {
    colliders.erase(std::remove(colliders.begin(), colliders.end(), this), colliders.end());
}

void Collider::shapeToCollider(const std::vector<float2>& points) {
    type = ColliderType::Shape;
    shape.points = points;
}

void Collider::rectToCollider(Rect argRect) {
    type = ColliderType::Rect;
    rect = argRect;
}

void Collider::triangleToCollider(Triangle argTriangle) {
    type = ColliderType::Triangle;
    triangle = argTriangle;
}

void Collider::circleToCollider(Circle argCircle) {
    type = ColliderType::Circle;
    circle = argCircle;
}

void Collider::spriteToCollider(const Sprite& sprite, float scale) {
    type = ColliderType::Rect;
    rect = { sprite.pos.x, sprite.pos.y, sprite.width * scale, sprite.height * scale };
}

void Collider::rawImageToCollider(const RawImage& image) {
    type = ColliderType::Rect;
    rect = { 0, 0, cast<float>(image.w), cast<float>(image.h) };
}

static bool checkRectRect(Rect r1, Rect r2) {
    return (r1.x < r2.x + r2.w &&
            r1.x + r1.w > r2.x &&
            r1.y < r2.y + r2.h &&
            r1.y + r1.h > r2.y);
}

static bool checkCircleCircle(Circle c1, Circle c2) {
    float dx = c1.pos.x - c2.pos.x;
    float dy = c1.pos.y - c2.pos.y;
    float distanceSq = dx * dx + dy * dy;
    float radiusSum = c1.radius + c2.radius;
    return distanceSq < (radiusSum * radiusSum);
}

static bool checkRectCircle(Rect r, Circle c) {
    float closestX = std::max(r.x, std::min(c.pos.x, r.x + r.w));
    float closestY = std::max(r.y, std::min(c.pos.y, r.y + r.h));

    float dx = c.pos.x - closestX;
    float dy = c.pos.y - closestY;

    return (dx * dx + dy * dy) < (c.radius * c.radius);
}

static bool checkTrianglePoint(Triangle t, float2 p) {
    auto sign = [](float2 p1, float2 p2, float2 p3) {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    };

    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(p, t.pointA, t.pointB);
    d2 = sign(p, t.pointB, t.pointC);
    d3 = sign(p, t.pointC, t.pointA);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

bool Collider::checkCollision(const Collider& other) const {
    if (type == ColliderType::None || other.type == ColliderType::None) return false;

    if (type == ColliderType::Rect && other.type == ColliderType::Rect) {
        return checkRectRect(rect, other.rect);
    }
    if (type == ColliderType::Circle && other.type == ColliderType::Circle) {
        return checkCircleCircle(circle, other.circle);
    }
    if (type == ColliderType::Rect && other.type == ColliderType::Circle) {
        return checkRectCircle(rect, other.circle);
    }
    if (type == ColliderType::Circle && other.type == ColliderType::Rect) {
        return checkRectCircle(other.rect, circle);
    }
    
    if (type == ColliderType::Triangle && other.type == ColliderType::Rect) {
        float2 center = { other.rect.x + other.rect.w/2, other.rect.y + other.rect.h/2 };
        return checkTrianglePoint(triangle, center);
    }

    return false;
}

void Collider::debug_drawColliders(SDL_Renderer* renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    for (auto* collider : colliders) {
        if (collider->type == ColliderType::None) continue;

        if (collider->type == ColliderType::Rect) {
            SDL_FRect fRect = { collider->rect.x, collider->rect.y, collider->rect.w, collider->rect.h };
            SDL_RenderRect(renderer, &fRect);
        } else if (collider->type == ColliderType::Circle) {
            // Draw circle as a rough approximation with lines
            const int segments = 16;
            for (int i = 0; i < segments; i++) {
                float a1 = (float)i * 2.0f * (float)M_PI / (float)segments;
                float a2 = (float)(i + 1) * 2.0f * (float)M_PI / (float)segments;
                SDL_RenderLine(renderer,
                               collider->circle.pos.x + cosf(a1) * collider->circle.radius,
                               collider->circle.pos.y + sinf(a1) * collider->circle.radius,
                               collider->circle.pos.x + cosf(a2) * collider->circle.radius,
                               collider->circle.pos.y + sinf(a2) * collider->circle.radius);
            }
        } else if (collider->type == ColliderType::Triangle) {
            SDL_RenderLine(renderer, collider->triangle.pointA.x, collider->triangle.pointA.y, collider->triangle.pointB.x, collider->triangle.pointB.y);
            SDL_RenderLine(renderer, collider->triangle.pointB.x, collider->triangle.pointB.y, collider->triangle.pointC.x, collider->triangle.pointC.y);
            SDL_RenderLine(renderer, collider->triangle.pointC.x, collider->triangle.pointC.y, collider->triangle.pointA.x, collider->triangle.pointA.y);
        } else if (collider->type == ColliderType::Shape) {
            if (collider->shape.points.size() > 1) {
                for (size_t i = 0; i < collider->shape.points.size(); i++) {
                    float2 p1 = collider->shape.points[i];
                    float2 p2 = collider->shape.points[(i + 1) % collider->shape.points.size()];
                    SDL_RenderLine(renderer, p1.x, p1.y, p2.x, p2.y);
                }
            }
        }
    }
}
