//
// Created by Stěpán Toman on 10.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_SPRITE_HPP
#define TRANSLUCENCEWORKSPACE_SPRITE_HPP

#include <T_Core.hpp>
#include <Application.hpp>

#include "Collider.hpp"
#include "Input.hpp"

class Sprite {

public:
    std::string filePath;
    float2 pos = {0, 0};
    float width = 0.0f;
    float height = 0.0f;
    float scale = 1.0f;
    bool useGravity = false;
    float gravityStrength = 980.0f;
    bool autoUpdatePhysics = true;

    explicit Sprite(Application& application, const std::string& argFilePath);
    ~Sprite();

    SDL_Surface* getSurface() const;
    SDL_Texture* getTexture() const;

    float2 getCenterPos();
    float2 getFeetPos();

    void update(float scale = 1.0f);
    void updatePhysics(float dt);
    void assignCollider(float scale = 1.0f);

    void applyGravity(float strength, float dt);
    void resolveCollision(const Collider& other, float scale = 1.0f);
    void wadMovement(float speed, float jumpStrength, float dt);
    void wadMovement(float speed, float dt) { wadMovement(speed, 300.0f, dt); }

    Collider& getCollider() { return collider; }
    
    float2 velocity = {0, 0};
    bool grounded = false;

    static const std::vector<Sprite*>& getSprites() { return sprites; }

private:
    static std::vector<Sprite*> sprites;
    Collider collider;
    Application& app;

    SDL_Surface* surface;
    SDL_Texture* texture;
    
};


#endif //TRANSLUCENCEWORKSPACE_SPRITE_HPP
