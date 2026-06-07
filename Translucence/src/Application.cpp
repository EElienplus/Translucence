//
// Created by Stěpán Toman on 04.05.2026.
//

#include "Application.hpp"
#include "EventSystem.hpp"
#include "Sprite.hpp"
#include "LayoutManager.hpp"
#include "Input.hpp"
#include <algorithm>

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

    MIX_DestroyMixer(mixer);

    MIX_Quit();
    TTF_Quit();
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
        SDL_SetRenderVSync(renderer, 1);
    }

    textEngine = TTF_CreateRendererTextEngine(renderer);

    if (fontPath == "default") fontPath = getDefaultFontPath();

    if (fontPath.empty()) {
        SDL_Log("Could not find a valid default font path!");
    }
    else {
        font = TTF_OpenFont(fontPath.c_str(), (float)baseFontSize);
        if (!font) {
            SDL_Log("TTF_OpenFont failed: %s", SDL_GetError());
        }
    }

    if (window) {
        SDL_GetWindowSize(window, &width, &height);
    } else {
        width = argWidth;
        height = argHeight;
    }
    initialWidth = width;
    initialHeight = height;
    title = argTitle;
    running = true;

    Input::setApplication(this);

    std::string iconPath = TRANSLUCENCE_RESOURCE_DIR "icon.png";
    setTitleIcon(iconPath);

    if (!MIX_Init()) {
        triggerError("Couldn't initialize SDL_mixer");
        return;
    }

    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!mixer) {
        triggerError("Couldn't create mixer");
        return;
    }

    if (window) SDL_ShowWindow(window);
}

std::string Application::getDefaultFontPath() {
    std::string path = TRANSLUCENCE_RESOURCE_DIR "defaultFont.ttf";
    if (fs::exists(path)) return path;

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
        if (fs::exists(p)) {
            path = p;
            break;
        }
    }
#endif

    return path;
}


SDL_Renderer* Application::getRenderer() {
    return renderer;
}

SDL_Window* Application::getWindow() {
    return window;
}

TTF_TextEngine* Application::getTextEngine() {
    return textEngine;
}

MIX_Mixer* Application::getMixer() {
    return mixer;
}

TTF_Font* Application::getFont() {
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

int Application::getInitialWidth() const {
    return initialWidth;
}

int Application::getInitialHeight() const {
    return initialHeight;
}

int Application::getFontSize() {
    return baseFontSize;
}

bool Application::isRunning() {
    return running;
}

int Application::getFPS() const {
    return currentFPS;
}

void Application::setRunning(bool argIsRunning) {
    running = argIsRunning;
}

void Application::setResizable(bool argValue) {
    SDL_SetWindowResizable(window, argValue);
}

bool Application::isResizable() const {
    if (window) {
        return (SDL_GetWindowFlags(window) & SDL_WINDOW_RESIZABLE) != 0;
    }
    return (windowFlags & SDL_WINDOW_RESIZABLE) != 0;
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

void Application::triggerError(const std::string& errorMsg) {
    print("TRANSLUCENCE ERROR: ");
    println(errorMsg);
    setRunning(false);
}

void Application::updateDeltaTime() {
    uint64_t currentTime = SDL_GetTicksNS();
    if (lastTime == 0) {
        lastTime = currentTime - 16666666; // Assume ~60fps for first frame
    }
    uint64_t deltaNS = currentTime - lastTime;
    lastTime = currentTime;

    float newDelta = static_cast<float>(deltaNS) / 1000000000.0f;

    // Cap delta time to 0.1s to prevent huge jumps after stalls
    if (newDelta > 0.1f) newDelta = 0.1f;
    
    deltaTime = newDelta;

    // Accurate FPS counter (updates once per second)
    fpsCount++;
    if (currentTime - lastFPSUpdateTime >= 1000000000) { // 1 second in ns
        currentFPS = fpsCount;
        fpsCount = 0;
        lastFPSUpdateTime = currentTime;
    }
}

float Application::getTime() {
    SDL_Time ticks;
    if (SDL_GetCurrentTime(&ticks)) {
        return static_cast<float>(ticks) / 1000000000.0f; // nanoseconds to seconds
    }

    return 0.0f;
}

std::string* Application::getTitle() {
    return &title;
}

void Application::setTitleIcon(std::string filePath) {
    SDL_Surface* icon = IMG_Load(filePath.c_str());
    SDL_SetWindowIcon(window, icon);
}

void Application::setFontPath(std::string argFontPath) {
    fontPath = argFontPath;
}

void Application::setAutoResizeEnabled(bool enabled) {
    autoResizeEnabled = enabled;
}

bool Application::isAutoResizeEnabled() const {
    return autoResizeEnabled;
}

void Application::registerLayoutManager(LayoutManager* lm) {
    if (std::find(registeredLayouts.begin(), registeredLayouts.end(), lm) == registeredLayouts.end()) {
        registeredLayouts.push_back(lm);
    }
}

void Application::unregisterLayoutManager(LayoutManager* lm) {
    auto it = std::find(registeredLayouts.begin(), registeredLayouts.end(), lm);
    if (it != registeredLayouts.end()) {
        registeredLayouts.erase(it);
    }
}

const std::vector<LayoutManager*>& Application::getLayoutManagers() const {
    return registeredLayouts;
}

void Application::setWindowFlags(SDL_WindowFlags argWindowFlags) {
    windowFlags = argWindowFlags;
}

void Application::setInitFlags(SDL_InitFlags argInitFlags) {
    initFlags = argInitFlags;
}

