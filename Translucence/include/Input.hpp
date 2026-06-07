//
// Created by Stěpán Tomant on 04.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_INPUTSYSTEM_HPP
#define TRANSLUCENCEWORKSPACE_INPUTSYSTEM_HPP

#include "T_Core.hpp"
#include <unordered_map>
#include <string>

class Input {
public:
    enum class Key : int {
        UNKNOWN = SDL_SCANCODE_UNKNOWN,
        SPACE = SDL_SCANCODE_SPACE,
        A = SDL_SCANCODE_A,
        B = SDL_SCANCODE_B,
        C = SDL_SCANCODE_C,
        D = SDL_SCANCODE_D,
        E = SDL_SCANCODE_E,
        F = SDL_SCANCODE_F,
        G = SDL_SCANCODE_G,
        H = SDL_SCANCODE_H,
        I = SDL_SCANCODE_I,
        J = SDL_SCANCODE_J,
        K = SDL_SCANCODE_K,
        L = SDL_SCANCODE_L,
        M = SDL_SCANCODE_M,
        N = SDL_SCANCODE_N,
        O = SDL_SCANCODE_O,
        P = SDL_SCANCODE_P,
        Q = SDL_SCANCODE_Q,
        R = SDL_SCANCODE_R,
        S = SDL_SCANCODE_S,
        T = SDL_SCANCODE_T,
        U = SDL_SCANCODE_U,
        V = SDL_SCANCODE_V,
        W = SDL_SCANCODE_W,
        X = SDL_SCANCODE_X,
        Y = SDL_SCANCODE_Y,
        Z = SDL_SCANCODE_Z,
        NUM_1 = SDL_SCANCODE_1,
        NUM_2 = SDL_SCANCODE_2,
        NUM_3 = SDL_SCANCODE_3,
        NUM_4 = SDL_SCANCODE_4,
        NUM_5 = SDL_SCANCODE_5,
        NUM_6 = SDL_SCANCODE_6,
        NUM_7 = SDL_SCANCODE_7,
        NUM_8 = SDL_SCANCODE_8,
        NUM_9 = SDL_SCANCODE_9,
        NUM_0 = SDL_SCANCODE_0,
        ESCAPE = SDL_SCANCODE_ESCAPE,
        ENTER = SDL_SCANCODE_RETURN,
        LSHIFT = SDL_SCANCODE_LSHIFT,
        RSHIFT = SDL_SCANCODE_RSHIFT,
        LCTRL = SDL_SCANCODE_LCTRL,
        RCTRL = SDL_SCANCODE_RCTRL,
        LGUI = SDL_SCANCODE_LGUI,
        RGUI = SDL_SCANCODE_RGUI,
        LEFT = SDL_SCANCODE_LEFT,
        RIGHT = SDL_SCANCODE_RIGHT,
        UP = SDL_SCANCODE_UP,
        DOWN = SDL_SCANCODE_DOWN,
        BACKSPACE = SDL_SCANCODE_BACKSPACE,
        DELETE = SDL_SCANCODE_DELETE,
        HOME = SDL_SCANCODE_HOME,
        END = SDL_SCANCODE_END,
    };
    enum class MouseButton : uint8_t {
        LEFT = SDL_BUTTON_LEFT,
        MIDDLE = SDL_BUTTON_MIDDLE,
        RIGHT = SDL_BUTTON_RIGHT,
        X1 = SDL_BUTTON_X1,
        X2 = SDL_BUTTON_X2
    };

    Input() = delete;

    static void beginFrame();
    static void processEvent(const SDL_Event &e);
    static void endFrame();

    static void clearTextInput();

    static bool isKeyDown(SDL_Scancode key);
    static bool isKeyPressed(SDL_Scancode key);
    static bool isKeyReleased(SDL_Scancode key);

    static bool isKeyDown(Key key);
    static bool isKeyPressed(Key key);
    static bool isKeyReleased(Key key);

    static int getNumberFromKey();
    static int getNumberFromKeyOffset();

    static bool isMouseButtonDown(uint8_t button);
    static bool isMouseButtonPressed(uint8_t button);
    static bool isMouseButtonReleased(uint8_t button);

    static bool isMouseClicked() {
        return isMouseButtonDown(static_cast<uint8_t>(MouseButton::LEFT));
    }

    static int getMouseX();
    static int getMouseY();
    static float2 getMousePos() {
        float x, y;
        SDL_GetMouseState(&x, &y);
        return {x, y};
    }
    static float getMouseWheelY();

    static void setApplication(class Application* app);

    static bool isMouseHoveringRect(float2 mousePos, Rect rect);
    static bool isMouseHoveringCircle(float2 mousePos, Circle circle) {
        const float dx = mousePos.x - circle.pos.x;
        const float dy = mousePos.y - circle.pos.y;

        return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
    }

    static const std::string &getLastTextInput();

    static void wasdMovement(float2& pos, float speed, float dt) {
        float2 moveDir(0, 0);
        if(Input::isKeyDown(Key::W)) {moveDir.y -= 1.0f;}
        if(Input::isKeyDown(Key::A)) {moveDir.x -= 1.0f;}
        if(Input::isKeyDown(Key::S)) {moveDir.y += 1.0f;}
        if(Input::isKeyDown(Key::D)) {moveDir.x += 1.0f;}

        if (moveDir.lengthSq() > 0) {
            moveDir = moveDir.normalized();
        }

        pos += moveDir * speed * dt;
    }

    /**
     * @brief Advanced WASD movement with velocity, acceleration and friction.
     * 
     * @param pos The position to update
     * @param velocity The velocity to update
     * @param speed Maximum movement speed
     * @param acceleration How fast the character reaches max speed
     * @param friction Deceleration rate when no input is provided
     * @param dt Delta time
     * @param normalizeDiagonal Whether to normalize diagonal movement speed
     */
    static void wasdMovement(float2& pos, float2& velocity, float speed, float acceleration, float friction, float dt, bool normalizeDiagonal = true) {
        float2 moveDir(0, 0);
        if(Input::isKeyDown(Key::W)) {moveDir.y -= 1.0f;}
        if(Input::isKeyDown(Key::A)) {moveDir.x -= 1.0f;}
        if(Input::isKeyDown(Key::S)) {moveDir.y += 1.0f;}
        if(Input::isKeyDown(Key::D)) {moveDir.x += 1.0f;}

        if (normalizeDiagonal && moveDir.lengthSq() > 0) {
            moveDir = moveDir.normalized();
        }

        if (moveDir.lengthSq() > 0) {
            // Snappy turning: if we are moving against our current velocity, accelerate faster
            float currentAcc = acceleration;
            if (velocity.length() > 0 && float2::dot(moveDir, velocity.normalized()) < 0) {
                currentAcc += friction * 2.0f; // More aggressive turn-around
            }
            velocity += moveDir * currentAcc * dt;
        } else {
            // Decelerate when no input - using higher friction for snappier stop
            float frictionAmount = friction * 2.5f * dt;
            if (velocity.length() < frictionAmount) {
                velocity = {0, 0};
            } else {
                velocity -= velocity.normalized() * frictionAmount;
            }
        }

        // Clamp velocity to max speed
        if (velocity.length() > speed) {
            velocity = velocity.normalized() * speed;
        }

        pos += velocity * dt;
    }


private:
    static std::unordered_map<SDL_Scancode, bool> currentKeys;
    static std::unordered_map<SDL_Scancode, bool> pressedKeys;
    static std::unordered_map<SDL_Scancode, bool> releasedKeys;

    static std::unordered_map<uint8_t, bool> currentButtons;
    static std::unordered_map<uint8_t, bool> pressedButtons;
    static std::unordered_map<uint8_t, bool> releasedButtons;

    static int mouseX;
    static int mouseY;
    static float wheelY;
    static std::string lastTextInput;
    static class Application* appInstance;
};

#endif //TRANSLUCENCEWORKSPACE_INPUTSYSTEM_HPP