//
// Created by Stěpán Toman on 02.06.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_UIPANEL_HPP
#define TRANSLUCENCEWORKSPACE_UIPANEL_HPP

#include <T_Core.hpp>

class Renderer;

class UI_Panel {
public:
    Rect area{0, 0, 100, 100};
    SDL_Color bgColor{0, 0, 0, 255};
    int paddingX{8};
    int paddingY{5};
    OutlineProperties outlineProperties{};

    // Manually set positions will be overwritten!

    void addButton(Renderer& renderer, Button& button, int id);
    void addSlider(Renderer& renderer, Slider& slider, int id);
    void addInputField(Renderer& renderer, InputField& inputField, int id);
    void addText(Renderer& renderer, String text, int textSize, int id);

    void resetLayout();

private:
    float cursorX = -1.0f;

    float2 setButtonPos(Button& button, int id);
    float2 setSliderPos(Slider& slider, int id);
    float2 setInputFieldPos(InputField& inputField, int id);
    float2 setTextPos(String text, int textsize, int id);


};
#endif //TRANSLUCENCEWORKSPACE_UIPANEL_HPP
