//
// Created by Stěpán Toman on 10.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_COLLIDER_HPP
#define TRANSLUCENCEWORKSPACE_COLLIDER_HPP

#include <T_Core.hpp>

#include "Application.hpp"

class Sprite;

enum class ColliderType {
    None,
    Shape,
    Rect,
    Triangle,
    Circle
};

class Collider {

public:
    Collider();
    ~Collider();

    void shapeToCollider(const std::vector<float2>& points);
    void rectToCollider(Rect rect);
    void triangleToCollider(Triangle triangle);
    void circleToCollider(Circle circle);
    void spriteToCollider(const Sprite& sprite, float scale = 1.0f);
    void rawImageToCollider(const RawImage& image);

    bool checkCollision(const Collider& other) const;

    ColliderType getType() const { return type; }
    Shape getShape() const { return shape; }
    Rect getRect() const { return rect; }
    Triangle getTriangle() const { return triangle; }
    Circle getCircle() const { return circle; }

    static void debug_drawColliders(SDL_Renderer* renderer, SDL_Color color);
    static const std::vector<Collider*>& getColliders() { return colliders; }
    static void makeScreenBorderCollider(Application& app);

private:
    static std::vector<Collider*> colliders;

    ColliderType type = ColliderType::None;
    
    Shape shape;
    Rect rect;
    Triangle triangle;
    Circle circle;

};


#endif //TRANSLUCENCEWORKSPACE_COLLIDER_HPP
