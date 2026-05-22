//
// Created by Stěpán Toman on 04.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_APPLICATION_HPP
#define TRANSLUCENCEWORKSPACE_APPLICATION_HPP

#include "T_Core.hpp"
#include <SDL3_mixer/SDL_mixer.h>


class Application {
public:

    Application() = default;
    ~Application();

    void create(int argWidth, int argHeight, std::string argTitle);

    SDL_Renderer* getRenderer();
    SDL_Window* getWindow();
    TTF_TextEngine* getTextEngine();
    MIX_Mixer* getMixer();
    TTF_Font* getFont();
    int getWidth() const;
    int getHeight() const;
    int getFontSize();
    bool isRunning();
    int getFPS() const;
    std::string* getTitle();
    void setTitleIcon(std::string filePath);
    void setFontPath(std::string argFontPath);
    void setWindowFlags(SDL_WindowFlags argWindowFlags);
    void setInitFlags(SDL_InitFlags argInitFlags);
    void setFontSize(int argFontSize);
    void setRunning(bool argIsRunning);
    void setResizable(bool argValue);
    std::string getFontPath();
    float getDeltaTime();
    void updateDeltaTime();
    float getTime();
    void update();
    void triggerError(const std::string& errorMsg);

private:
    SDL_Renderer* renderer = nullptr;
    SDL_Window* window = nullptr;
    TTF_TextEngine* textEngine = nullptr;
    MIX_Mixer* mixer;
    TTF_Font* font = nullptr;
    std::string fontPath = "default";
    SDL_InitFlags initFlags = SDL_INIT_VIDEO;
    SDL_WindowFlags windowFlags = SDL_WINDOW_RESIZABLE;
    int width;
    int height;
    std::string title;
    std::string getDefaultFontPath();
    int baseFontSize = 30;
    uint64_t lastTime = 0;
    float deltaTime = 0.0f;
    bool running = true;
};


#endif //TRANSLUCENCEWORKSPACE_APPLICATION_HPP
