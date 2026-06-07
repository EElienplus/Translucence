//
// Created by Stěpán Toman on 02.06.2026.
//

#include "UiPanel.hpp"
#include "Renderer.hpp"

void UI_Panel::addButton(Renderer& renderer, Button& button, int id) {
    if (cursorX < 0) resetLayout();
    float2 pos = setButtonPos(button, id);
    button.rect.x = pos.x;
    button.rect.y = pos.y;
    renderer.drawButton(button);
    cursorX = button.rect.x + button.rect.w + (float)paddingX;
}
void UI_Panel::addSlider(Renderer& renderer, Slider& slider, int id) {
    if (cursorX < 0) resetLayout();
    float2 pos = setSliderPos(slider, id);
    slider.rect.x = pos.x;
    slider.rect.y = pos.y;
    renderer.drawSlider(slider);
    cursorX = slider.rect.x + slider.rect.w + (float)paddingX;
}
void UI_Panel::addInputField(Renderer& renderer, InputField& inputField, int id) {
    if (cursorX < 0) resetLayout();
    float2 pos = setInputFieldPos(inputField, id);
    inputField.rect.x = pos.x;
    inputField.rect.y = pos.y;
    renderer.drawInputField(inputField);
    cursorX = inputField.rect.x + inputField.rect.w + (float)paddingX;
}
void UI_Panel::addText(Renderer& renderer, std::string text, int textSize, int id) {
    if (cursorX < 0) resetLayout();
    float2 size = getTextSize(renderer.getApp().getFont(), text, textSize);
    float2 pos = setTextPos(text, textSize, id);
    // Center vertically
    pos.y = area.y + (area.h - size.y) / 2.0f;
    renderer.drawText(text, pos, Color::TextPrimary, textSize);
    cursorX = pos.x + size.x + (float)paddingX;
}

void UI_Panel::resetLayout() {
    cursorX = area.x + (float)paddingX;
}

// The parentheses just make it more readable for me
float2 UI_Panel::setButtonPos(Button& button, int id) {
    (void)id;
    float2 pos;
    // Dynamic scaling/centering: use cursor if it's the next element, 
    // or calculate based on slots if id is large.
    // For now, let's prioritize the cursor to avoid overlaps.
    pos.x = cursorX;
    pos.y = area.y + (area.h - button.rect.h) / 2.0f;
    return pos;
}
float2 UI_Panel::setSliderPos(Slider& slider, int id) {
    (void)id;
    float2 pos;
    pos.x = cursorX;
    pos.y = area.y + (area.h - slider.rect.h) / 2.0f;
    return pos;
}
float2 UI_Panel::setInputFieldPos(InputField& inputField, int id) {
    (void)id;
    float2 pos;
    pos.x = cursorX;
    pos.y = area.y + (area.h - inputField.rect.h) / 2.0f;
    return pos;
}
float2 UI_Panel::setTextPos(std::string text, int textsize, int id) {
    (void)text;
    (void)textsize;
    (void)id;
    float2 pos;
    pos.x = cursorX;
    // Vertical centering is handled in addText since we need the size from getTextSize
    pos.y = area.y + (float)paddingY;
    return pos;
}
