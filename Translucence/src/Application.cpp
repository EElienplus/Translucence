//
// Created by Stěpán Toman on 04.05.2026.
//

#include "Application.hpp"
#include "EventSystem.hpp"
#include "Sprite.hpp"


Application::~Application() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }

    if (textEngine) {
        TTF_DestroyRendererTextEngine(textEngine);
        textEngine = nullptr;
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    running = false;

    SDL_Quit();
}
void Application::create(int argWidth, int argHeight, std::string argTitle) {
    println("");
    if (!(initFlags & SDL_INIT_VIDEO)) {
        initFlags = (SDL_InitFlags)(initFlags | SDL_INIT_VIDEO);
    }
    SDL_Init(initFlags);

    if (!TTF_Init()) {
        SDL_Log("TTF_Init failed: %s", SDL_GetError());
    }
    window = SDL_CreateWindow(argTitle.c_str(), argWidth, argHeight, windowFlags);
    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    }

    textEngine = TTF_CreateRendererTextEngine(renderer);

    if (fontPath == "default") fontPath = getDefaultFontPath();

    if (fontPath.empty()) {
        SDL_Log("Could not find a valid default font path!");
    } else {
        font = TTF_OpenFont(fontPath.c_str(), (float)baseFontSize);
        if (!font) {
            SDL_Log("TTF_OpenFont failed: %s", SDL_GetError());
        }
    }

    width = argWidth;
    height = argHeight;
    title = argTitle;
    running = true;

    setTitleIcon("../../Translucence/resources/icon.png");

    if (window) SDL_ShowWindow(window);
}

std::string Application::getDefaultFontPath() {
    std::string path = "";
    const char* base_path_ptr = SDL_GetBasePath();

    if (base_path_ptr) {
        std::string base(base_path_ptr);
        std::string tryPath = base + "../../Translucence/resouces/defaultFont.ttf";
        if (fs::exists(tryPath)) return tryPath;

        tryPath = base + "../../Translucence/resources/defaultFont.ttf";
        if (fs::exists(tryPath)) return tryPath;

        tryPath = base + "../../../Translucence/resouces/defaultFont.ttf";
        if (fs::exists(tryPath)) return tryPath;
    }

#if defined(__APPLE__)
    path = "/System/Library/Fonts/Helvetica.ttc";
#elif defined(_WIN32)
    const char* winDir = std::getenv("WINDIR");
    path = (winDir ? std::string(winDir) : "C:\\Windows") + "\\Fonts\\arial.ttf";
#elif defined(__linux__)
    std::string paths[] = {
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf"
    };
    for (const std::string& p : paths) {
        if (fs::exists(p)) { path = p; break; }
    }
#endif

    return path;
}


SDL_Renderer * Application::getRenderer() const {
    return renderer;
}

SDL_Window * Application::getWindow() const {
    return window;
}

TTF_TextEngine * Application::getTextEngine() const {
    return textEngine;
}

TTF_Font * Application::getFont() {
    return font;
}

int Application::getWidth() const {
    if (window) {
        int w = 0, h = 0;
        SDL_GetWindowSize(window, &w, &h);
        if (w > 0) return w;
    }
    return width;
}

int Application::getHeight() const {
    if (window) {
        int w = 0, h = 0;
        SDL_GetWindowSize(window, &w, &h);
        if (h > 0) return h;
    }
    return height;
}

int Application::getFontSize() {
    return baseFontSize;
}

bool Application::isRunning() {
    return running;
}

int Application::getFPS() const {
    if (deltaTime > 0) {
        return static_cast<int>(1.0f / deltaTime);
    }
    return 0;
}

void Application::setRunning(bool argIsRunning) {
    running = argIsRunning;
}

void Application::setResizable(bool argValue) {
    SDL_SetWindowResizable(window, argValue);
}

std::string Application::getFontPath() {
    if (fontPath != "default") return fontPath;
    return getDefaultFontPath();
}

void Application::setFontSize(int argFontSize) {
    baseFontSize = argFontSize;
}

float Application::getDeltaTime() {
    return deltaTime;
}
void Application::update() {
    updateDeltaTime();
    for (auto* sprite : Sprite::getSprites()) {
        sprite->updatePhysics(deltaTime);
    }
}

void Application::updateDeltaTime() {
    uint64_t currentTime = SDL_GetTicksNS();
    if (lastTime == 0) lastTime = currentTime;
    uint64_t deltaNS = currentTime - lastTime;
    lastTime = currentTime;

    float newDelta = static_cast<float>(deltaNS) / 1000000000.0f;
    
    // Simple Exponential Moving Average for smoothing
    // 0.8 current, 0.2 previous
    if (deltaTime == 0.0f) {
        deltaTime = newDelta;
    } else {
        deltaTime = deltaTime * 0.2f + newDelta * 0.8f;
    }
}
float Application::getTime() {
    SDL_Time ticks;
    if (SDL_GetCurrentTime(&ticks)) {
        return static_cast<float>(ticks) / 1000000000.0f; // nanoseconds to seconds
    }

    return 0.0f;
}

std::string *Application::getTitle() {
    return &title;
}

void Application::setTitleIcon(std::string filePath) {
    SDL_Surface* icon = IMG_Load(filePath.c_str());
    SDL_SetWindowIcon(window, icon);
}

void Application::setFontPath(std::string argFontPath) {
    fontPath = argFontPath;
}

void Application::setWindowFlags(SDL_WindowFlags argWindowFlags) {
    windowFlags = argWindowFlags;
}

void Application::setInitFlags(SDL_InitFlags argInitFlags) {
    initFlags = argInitFlags;
}

