//
// Created by Stěpán Toman on 04.05.2026.
//

#include "Input.hpp"

// static storage
std::unordered_map<SDL_Scancode, bool> Input::currentKeys;
std::unordered_map<SDL_Scancode, bool> Input::pressedKeys;
std::unordered_map<SDL_Scancode, bool> Input::releasedKeys;
std::unordered_map<uint8_t, bool> Input::currentButtons;
std::unordered_map<uint8_t, bool> Input::pressedButtons;
std::unordered_map<uint8_t, bool> Input::releasedButtons;
std::string Input::lastTextInput;
int Input::mouseX = 0;
int Input::mouseY = 0;
float Input::wheelY = 0.0f;

void Input::beginFrame() {
    pressedKeys.clear();
    releasedKeys.clear();
    pressedButtons.clear();
    releasedButtons.clear();
    lastTextInput.clear();
    wheelY = 0.0f;
}

void Input::processEvent(const SDL_Event &e) {
    switch (e.type) {
        case SDL_EVENT_KEY_DOWN:
            currentKeys[e.key.scancode] = true;
            pressedKeys[e.key.scancode] = true;
            break;

        case SDL_EVENT_KEY_UP:
            currentKeys[e.key.scancode] = false;
            releasedKeys[e.key.scancode] = true;
            break;

        case SDL_EVENT_TEXT_INPUT:
            if (e.text.text) {
                lastTextInput += e.text.text;
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            currentButtons[e.button.button] = true;
            pressedButtons[e.button.button] = true;
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            currentButtons[e.button.button] = false;
            releasedButtons[e.button.button] = true;
            break;

        case SDL_EVENT_MOUSE_MOTION:
            mouseX = static_cast<int>(e.motion.x);
            mouseY = static_cast<int>(e.motion.y);
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            wheelY += static_cast<float>(e.wheel.y);
            break;

        default:
            break;
    }
}

void Input::endFrame() {
}

void Input::clearTextInput() {
    lastTextInput.clear();
}

bool Input::isKeyDown(SDL_Scancode key) {
    auto it = currentKeys.find(key);
    return it != currentKeys.end() && it->second;
}

bool Input::isKeyPressed(SDL_Scancode key) {
    auto it = pressedKeys.find(key);
    return it != pressedKeys.end() && it->second;
}

bool Input::isKeyReleased(SDL_Scancode key) {
    auto it = releasedKeys.find(key);
    return it != releasedKeys.end() && it->second;
}

// Key overloads
bool Input::isKeyDown(Key key) {
    return isKeyDown(static_cast<SDL_Scancode>(key));
}

bool Input::isKeyPressed(Key key) {
    return isKeyPressed(static_cast<SDL_Scancode>(key));
}

bool Input::isKeyReleased(Key key) {
    return isKeyReleased(static_cast<SDL_Scancode>(key));
}

int Input::getNumberFromKey() {
    if (Input::isKeyDown(Input::Key::NUM_1)) return 1;
    else if (Input::isKeyDown(Input::Key::NUM_2)) return 2;
    else if (Input::isKeyDown(Input::Key::NUM_3)) return 3;
    else if (Input::isKeyDown(Input::Key::NUM_4)) return 4;
    else if (Input::isKeyDown(Input::Key::NUM_5)) return 5;
    else if (Input::isKeyDown(Input::Key::NUM_6)) return 6;
    else if (Input::isKeyDown(Input::Key::NUM_7)) return 7;
    else if (Input::isKeyDown(Input::Key::NUM_8)) return 8;
    else if (Input::isKeyDown(Input::Key::NUM_9)) return 9;
    else if (Input::isKeyDown(Input::Key::NUM_0)) return 0;
    return -1;
}

int Input::getNumberFromKeyOffset() {
    if (Input::isKeyDown(Input::Key::NUM_1)) return 0;
    else if (Input::isKeyDown(Input::Key::NUM_2)) return 1;
    else if (Input::isKeyDown(Input::Key::NUM_3)) return 2;
    else if (Input::isKeyDown(Input::Key::NUM_4)) return 3;
    else if (Input::isKeyDown(Input::Key::NUM_5)) return 4;
    else if (Input::isKeyDown(Input::Key::NUM_6)) return 5;
    else if (Input::isKeyDown(Input::Key::NUM_7)) return 6;
    else if (Input::isKeyDown(Input::Key::NUM_8)) return 7;
    else if (Input::isKeyDown(Input::Key::NUM_9)) return 8;
    else if (Input::isKeyDown(Input::Key::NUM_0)) return 9;
    return -1;
}

bool Input::isMouseButtonDown(uint8_t button) {
    auto it = currentButtons.find(button);
    return it != currentButtons.end() && it->second;
}

bool Input::isMouseButtonPressed(uint8_t button) {
    auto it = pressedButtons.find(button);
    return it != pressedButtons.end() && it->second;
}

bool Input::isMouseButtonReleased(uint8_t button) {
    auto it = releasedButtons.find(button);
    return it != releasedButtons.end() && it->second;
}

int Input::getMouseX() { return mouseX; }
int Input::getMouseY() { return mouseY; }

float Input::getMouseWheelY() { return wheelY; }

const std::string &Input::getLastTextInput() { return lastTextInput; }

