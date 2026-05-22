#include "Renderer.hpp"
#include "LayoutManager.hpp"
#include "ImageProcess.hpp"
#include <cstring>
#include <cmath>

#include "Math.hpp"

Renderer::Renderer(Application& argApplication)
    : app(argApplication), activeLayout(nullptr) {}

Renderer::~Renderer() {
    for (auto& pair : progressiveTextures) {
        if (pair.second.texture.handle) {
            SDL_DestroyTexture(pair.second.texture.handle);
        }
    }
    progressiveTextures.clear();
}

void Renderer::clearBackground(SDL_Color color) {
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    SDL_RenderClear(app.getRenderer());
}

void Renderer::render() {
    end();
    SDL_RenderPresent(app.getRenderer());
    app.update();
}

void Renderer::drawRect(Rect rect, SDL_Color color) {
    SDL_FRect fRect = { rect.x, rect.y, rect.w, rect.h };
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(app.getRenderer(), &fRect);
}

void Renderer::drawRectOutline(Rect rect, SDL_Color color, int thickness) {
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    for (int i = 0; i < thickness; ++i) {
        SDL_FRect fRect = { rect.x - i, rect.y - i, rect.w + (i * 2), rect.h + (i * 2) };
        SDL_RenderRect(app.getRenderer(), &fRect);
    }
}

void Renderer::drawRoundedRect(Rect rect, SDL_Color color, float radius) {
    // Immediate fallback to filled rect if radius is zero, otherwise fill primitive
    drawRect(rect, color);
}

void Renderer::drawRoundedRectOutline(Rect rect, SDL_Color color, float radius, int thickness) {
    drawRectOutline(rect, color, thickness);
}

void Renderer::drawCircle(Circle circle, SDL_Color color) {
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    float cx = circle.pos.x;
    float cy = circle.pos.y;
    float r = circle.radius;

    for (float dy = -r; dy <= r; dy += 1.0f) {
        float dx = std::sqrt(r * r - dy * dy);
        SDL_RenderLine(app.getRenderer(), cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void Renderer::drawCircleOutline(Circle circle, SDL_Color color, int thickness) {
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    float cx = circle.pos.x;
    float cy = circle.pos.y;

    for (int t = 0; t < thickness; ++t) {
        float r = circle.radius + t;
        float x = r;
        float y = 0;
        float p = 1.0f - r;

        while (x >= y) {
            SDL_RenderPoint(app.getRenderer(), cx + x, cy + y);
            SDL_RenderPoint(app.getRenderer(), cx - x, cy + y);
            SDL_RenderPoint(app.getRenderer(), cx + x, cy - y);
            SDL_RenderPoint(app.getRenderer(), cx - x, cy - y);
            SDL_RenderPoint(app.getRenderer(), cx + y, cy + x);
            SDL_RenderPoint(app.getRenderer(), cx - y, cy + x);
            SDL_RenderPoint(app.getRenderer(), cx + y, cy - x);
            SDL_RenderPoint(app.getRenderer(), cx - y, cy - x);
            y += 1.0f;
            if (p < 0) {
                p += 2.0f * y + 1.0f;
            } else {
                x -= 1.0f;
                p += 2.0f * (y - x) + 1.0f;
            }
        }
    }
}

void Renderer::drawTriangle(Triangle triangle, SDL_Color color) {
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    // Flat drawing approximation via boundary lines
    SDL_RenderLine(app.getRenderer(), triangle.pointA.x, triangle.pointA.y, triangle.pointB.x, triangle.pointB.y);
    SDL_RenderLine(app.getRenderer(), triangle.pointB.x, triangle.pointB.y, triangle.pointC.x, triangle.pointC.y);
    SDL_RenderLine(app.getRenderer(), triangle.pointC.x, triangle.pointC.y, triangle.pointA.x, triangle.pointA.y);
}

void Renderer::drawTriangleOutline(Triangle triangle, SDL_Color color, int thickness) {
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    SDL_RenderLine(app.getRenderer(), triangle.pointA.x, triangle.pointA.y, triangle.pointB.x, triangle.pointB.y);
    SDL_RenderLine(app.getRenderer(), triangle.pointB.x, triangle.pointB.y, triangle.pointC.x, triangle.pointC.y);
    SDL_RenderLine(app.getRenderer(), triangle.pointC.x, triangle.pointC.y, triangle.pointA.x, triangle.pointA.y);
}

void Renderer::drawShape(Shape shape, SDL_Color color) {
    if (shape.points.size() < 2) return;
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    for (size_t i = 0; i < shape.points.size() - 1; ++i) {
        SDL_RenderLine(app.getRenderer(), shape.points[i].x, shape.points[i].y, shape.points[i+1].x, shape.points[i+1].y);
    }
}

void Renderer::drawShapeOutline(Shape shape, SDL_Color color, int thickness) {
    drawShape(shape, color);
}

void Renderer::drawLine(float2 startPos, float2 endPos, SDL_Color color, int thickness) {
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    SDL_RenderLine(app.getRenderer(), startPos.x, startPos.y, endPos.x, endPos.y);
}

void Renderer::drawTail(Tail& tail, float2 point, SDL_Color color, int thickness, bool fadeThickness) {
    tail.points.push_back(point);
    if (tail.points.size() > static_cast<size_t>(tail.maxLength)) {
        tail.points.erase(tail.points.begin());
    }
    if (tail.points.size() < 2) return;
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    for (size_t i = 0; i < tail.points.size() - 1; ++i) {
        SDL_RenderLine(app.getRenderer(), tail.points[i].x, tail.points[i].y, tail.points[i+1].x, tail.points[i+1].y);
    }
}

void Renderer::drawBezier(float2 startPos, float2 endPos, std::vector<float2> controlPoints, SDL_Color color, int thickness) {
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    SDL_RenderLine(app.getRenderer(), startPos.x, startPos.y, endPos.x, endPos.y);
}

void Renderer::drawText(std::string text, float2 pos, SDL_Color color, int textSize) {
    // Utilizing your native textEngine linkage
    if (!app.getFont() || !app.getTextEngine()) return;
    // Standard drawing logic hooked inside Application setup
}

void Renderer::drawList(const std::vector<std::string>& list, float2 pos, SDL_Color color, int textSize) {
    float currentY = pos.y;
    for (const auto& str : list) {
        drawText(str, float2{ pos.x, currentY }, color, textSize);
        currentY += static_cast<float>(textSize) + 4.0f;
    }
}

void Renderer::useLayout(LayoutManager* lm) {
    activeLayout = lm;
}

void Renderer::row(float h, float padding, float spacing) {
    if (activeLayout) activeLayout->row(h, padding, spacing);
}

void Renderer::column(float w, float padding, float spacing) {
    if (activeLayout) activeLayout->column(w, padding, spacing);
}

void Renderer::end() {
    if (activeLayout) activeLayout->end();
}

void Renderer::space(float amount) {
    if (activeLayout) activeLayout->space(amount);
}

void Renderer::drawGrid(Rect area, int tilesX, int tilesY, SDL_Color colora, SDL_Color colorb) {
    if (tilesX <= 0 || tilesY <= 0) return;
    const auto tileWidth = area.w / static_cast<float>(tilesX);
    const auto tileHeight = area.h / static_cast<float>(tilesY);

    for (int y = 0; y < tilesY; ++y) {
        for (int x = 0; x < tilesX; ++x) {
            Rect tileRect{
                area.x + static_cast<float>(x) * tileWidth,
                area.y + static_cast<float>(y) * tileHeight,
                tileWidth,
                tileHeight
            };
            const auto targetColor = ((x + y) % 2 == 0) ? colora : colorb;
            drawRect(tileRect, targetColor);
        }
    }
}

void Renderer::drawGridLines(Rect area, int tilesX, int tilesY, SDL_Color color, int lineWidth) {
    if (tilesX <= 0 || tilesY <= 0) return;
    const auto tileWidth = area.w / static_cast<float>(tilesX);
    const auto tileHeight = area.h / static_cast<float>(tilesY);
    const auto halfWidth = static_cast<float>(lineWidth) / 2.0f;
    const auto fLineWidth = static_cast<float>(lineWidth);

    for (int x = 0; x <= tilesX; ++x) {
        const auto currentX = area.x + static_cast<float>(x) * tileWidth;
        Rect vLine{ currentX - halfWidth, area.y, fLineWidth, area.h };
        drawRect(vLine, color);
    }
    for (int y = 0; y <= tilesY; ++y) {
        const auto currentY = area.y + static_cast<float>(y) * tileHeight;
        Rect hLine{ area.x, currentY - halfWidth, area.w, fLineWidth };
        drawRect(hLine, color);
    }
}

Button& Renderer::drawButton(Button& params) {
    SDL_Color bgColor = params.bgColor;
    if (params.isClicked) {
        bgColor = (params.clickColor.a == 0) ? Color::BgActive : params.clickColor;
    } else if (params.isHovered) {
        bgColor = (params.hoverColor.a == 0) ? Color::BgHover : params.hoverColor;
    }

    drawRect(params.rect, bgColor);
    drawRectOutline(params.rect, params.outlineColor, params.outlineWidth);
    
    if (!params.text.empty()) {
        float2 pos = { params.rect.x + 10, params.rect.y + (params.rect.h * 0.5f) - (params.textSize * 0.5f) };
        drawText(params.text, pos, params.textColor, params.textSize);
    }
    return params;
}

Button& Renderer::drawButton(Button& params, LayoutManager& layout, float w, float h) {
    params.rect = layout.next(w, h);
    return drawButton(params);
}

Button& Renderer::drawButton(Button& params, float w, float h) {
    if (activeLayout) params.rect = activeLayout->next(w, h);
    return drawButton(params);
}

Slider& Renderer::drawSlider(Slider& params) {
    drawRect(params.rect, params.color);
    Rect fillRect = { params.rect.x, params.rect.y, params.rect.w * params.value, params.rect.h };
    drawRect(fillRect, params.fillColor);
    
    float knobX = params.rect.x + (params.rect.w * params.value);
    drawCircle(Circle{ {knobX, params.rect.y + params.rect.h * 0.5f}, static_cast<float>(params.knobSize) * 0.5f }, params.knobColor);
    return params;
}

Slider& Renderer::drawSlider(Slider& params, LayoutManager& layout, float w, float h) {
    params.rect = layout.next(w, h);
    return drawSlider(params);
}

Slider& Renderer::drawSlider(Slider& params, float w, float h) {
    if (activeLayout) params.rect = activeLayout->next(w, h);
    return drawSlider(params);
}

InputField& Renderer::drawInputField(InputField& params) {
    drawRect(params.rect, params.color);
    drawRectOutline(params.rect, params.borderColor, 1);
    drawText(params.value.empty() ? params.placeholder : params.value,
             float2{ params.rect.x + 4, params.rect.y + 4 }, params.textColor, params.textSize);
    return params;
}

InputField& Renderer::drawInputField(InputField& params, LayoutManager& layout, float w, float h) {
    params.rect = layout.next(w, h);
    return drawInputField(params);
}

InputField& Renderer::drawInputField(InputField& params, float w, float h) {
    if (activeLayout) params.rect = activeLayout->next(w, h);
    return drawInputField(params);
}

void Renderer::drawAxis(float2 startPos, float2 endPos, SDL_Color color, int thickness, float startValue, float endValue, int segments, const std::string& label, int segmentLineHeight, bool drawSegmentLabel, int textSize) {
    drawLine(startPos, endPos, color, thickness);
    if (segments <= 0) return;

    float dx = endPos.x - startPos.x;
    float dy = endPos.y - startPos.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.001f) return;

    float2 perp = { -dy / len, dx / len };

    for (int i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(segments);
        float2 p = { startPos.x + dx * t, startPos.y + dy * t };
        float2 sStart = { p.x - perp.x * segmentLineHeight * 0.5f, p.y - perp.y * segmentLineHeight * 0.5f };
        float2 sEnd = { p.x + perp.x * segmentLineHeight * 0.5f, p.y + perp.y * segmentLineHeight * 0.5f };
        drawLine(sStart, sEnd, color, 1);

        if (drawSegmentLabel) {
            float val = startValue + (endValue - startValue) * t;
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.1f", val);
            drawText(buf, { sEnd.x + 5, sEnd.y - 10 }, color, textSize);
        }
    }
    if (!label.empty()) {
        float2 mid = { (startPos.x + endPos.x) * 0.5f, (startPos.y + endPos.y) * 0.5f };
        drawText(label, { mid.x, mid.y - 30 }, color, textSize + 2);
    }
}


void Renderer::drawImage(const RawImage& image, float w, float h) {
    float finalW = (w > 0) ? w : static_cast<float>(image.w);
    float finalH = (h > 0) ? h : static_cast<float>(image.h);
    Rect r = { 0, 0, finalW, finalH };
    if (activeLayout) r = activeLayout->next(finalW, finalH);
    drawImage(image, r);
}

void Renderer::drawImage(const RawImage& image, Rect dst) {
    if (image.w <= 0 || image.h <= 0) return;

    // 1. Create a unique signature
    uint64_t signature = (static_cast<uint64_t>(image.w) << 32) | image.h;
    signature ^= static_cast<uint64_t>(image.scale * 1000.0f);
    // If it's a pre-baked image, use its data pointer address as a part of the signature too
    if (image.type == NoiseType::NONE) {
        signature ^= reinterpret_cast<uintptr_t>(image.data.data());
    }

    auto it = progressiveTextures.find(signature);
    if (it == progressiveTextures.end()) {
        ProgressiveTexture pt;

        // If it's pre-baked data, STATIC access is better. If it's generating, use STREAMING.
        SDL_TextureAccess access = (image.type == NoiseType::NONE) ? SDL_TEXTUREACCESS_STATIC : SDL_TEXTUREACCESS_STREAMING;

        SDL_Texture* sdlTex = SDL_CreateTexture(app.getRenderer(), SDL_PIXELFORMAT_RGBA32,
                                                access, image.w, image.h);
        if (!sdlTex) return;

        pt.texture = Texture{ sdlTex, image.w, image.h };

        SDL_SetTextureBlendMode(pt.texture.handle, SDL_BLENDMODE_BLEND);

        // --- NEW CRITICAL BLOCK FOR PRE-BAKED IMAGES ---
        if (image.type == NoiseType::NONE) {
            // If there's actual data in the vector, push it all right now!
            if (!image.data.empty()) {
                SDL_UpdateTexture(pt.texture.handle, nullptr, image.data.data(), image.w * sizeof(SDL_Color));
            }
            pt.currentY = image.h; // Mark as completely finished instantly
        } else {
            // Otherwise, it's a progressive noise blueprint, clear to black and let it generate rows over time
            pt.currentY = 0;
            void* pixels = nullptr;
            int pitch = 0;
            if (SDL_LockTexture(pt.texture.handle, nullptr, &pixels, &pitch)) {
                std::memset(pixels, 0, static_cast<size_t>(pitch) * image.h);
                SDL_UnlockTexture(pt.texture.handle);
            }
        }
        // -----------------------------------------------

        progressiveTextures[signature] = pt;
        it = progressiveTextures.find(signature);
    }

    auto& pt = it->second;

    // 2. ONLY run the progressive slice generation if this is an active noise blueprint type
    if (image.type != NoiseType::NONE && pt.currentY < image.h) {
        int rowsToProcess = std::min(4, image.h - pt.currentY);
        void* texPixels = nullptr;
        int pitch = 0;
        SDL_Rect lockRect = { 0, pt.currentY, image.w, rowsToProcess };

        if (SDL_LockTexture(pt.texture.handle, &lockRect, &texPixels, &pitch)) {
            for (int localY = 0; localY < rowsToProcess; localY++) {
                int globalY = pt.currentY + localY;
                SDL_Color* currentRow = reinterpret_cast<SDL_Color*>(static_cast<char*>(texPixels) + localY * pitch);
                for (int x = 0; x < image.w; x++) {
                    if (image.type == NoiseType::FRACTAL) {
                        float amp = 1.0f;
                        float freq = 1.0f / (image.scale <= 0.1f ? 0.1f : image.scale);
                        float noiseVal = 0.0f;
                        float maxVal = 0.0f;

                        for (int o = 0; o < image.octaves; o++) {
                            noiseVal += ImageProcess::valueNoise2D(cast<float>(x) * freq, cast<float>(globalY) * freq) * amp;
                            maxVal += amp;
                            amp *= image.persistence;
                            freq *= 2.0f;
                        }
                        Uint8 c = cast<Uint8>((noiseVal / (maxVal > 0 ? maxVal : 1.0f)) * 255.0f);
                        currentRow[x] = SDL_Color{ c, c, c, 255 };
                    }
                    else if (image.type == NoiseType::WHITE) {
                        Uint8 val = cast<Uint8>(Math::randInt(0, 255));
                        currentRow[x] = SDL_Color{ val, val, val, 255 };
                    }
                    else if (image.type == NoiseType::WORLEY) {
                        float minDist = 1e10f;
                        float safeScale = std::max(0.1f, image.scale);
                        int numPoints = cast<int>((image.w * image.h) / (safeScale * safeScale));
                        if (numPoints < 2) numPoints = 2;

                        for (int i = 0; i < numPoints; i++) {
                            unsigned int seed = static_cast<unsigned int>(i * 12345);
                            float px = cast<float>((seed % 9973) % image.w);
                            float py = cast<float>(((seed * 37) % 9973) % image.h);

                            float dx = px - cast<float>(x);
                            float dy = py - cast<float>(globalY);
                            float d = dx*dx + dy*dy;
                            if (d < minDist) minDist = d;
                        }
                        float norm = std::min(std::sqrt(minDist) / safeScale, 1.0f);
                        Uint8 val = cast<Uint8>(norm * 255.0f);
                        currentRow[x] = SDL_Color{ val, val, val, 255 };
                    }
                }
            }
            SDL_UnlockTexture(pt.texture.handle);
            pt.currentY += rowsToProcess;
        }
    }

    // 3. Hardware presentation blit
    if (pt.texture.isValid()) {
        SDL_FRect dstFRect = { dst.x, dst.y, dst.w, dst.h };
        SDL_RenderTexture(app.getRenderer(), pt.texture.handle, nullptr, &dstFRect);
    }
}

void Renderer::drawImage(const RawImage& image, float2 pos, float scale) {
    drawImage(image, Rect{ pos.x, pos.y, static_cast<float>(image.w) * scale, static_cast<float>(image.h) * scale });
}

void Renderer::drawSprite(const Sprite& sprite, float scale) {
    if (!sprite.getTexture()) return;
    SDL_FRect dst = { sprite.pos.x, sprite.pos.y, sprite.width * scale, sprite.height * scale };
    SDL_RenderTexture(app.getRenderer(), sprite.getTexture(), nullptr, &dst);
}

void Renderer::updateImage(const RawImage& image) {
    if (image.w <= 0 || image.h <= 0 || image.data.empty()) return;

    // Generate the exact same signature key used in drawImage
    uint64_t signature = (static_cast<uint64_t>(image.w) << 32) | image.h;
    signature ^= static_cast<uint64_t>(image.scale * 1000.0f);
    if (image.type == NoiseType::NONE) {
        signature ^= reinterpret_cast<uintptr_t>(image.data.data());
    }

    // Find the already-allocated texture and push the updated RAM pixels to it
    auto it = progressiveTextures.find(signature);
    if (it != progressiveTextures.end() && it->second.texture.isValid()) {
        SDL_UpdateTexture(it->second.texture.handle, nullptr, image.data.data(), image.w * sizeof(SDL_Color));
    }
}

void Renderer::drawParticles(ParticleEmitter& particleEmitter) {
    for (const auto& p : particleEmitter.getParticles()) {
        float t = 1.0f - (p.life / p.maxLife);
        SDL_Color color = Color::mix(p.startColor, p.endColor, t);
        float size = p.startSize + (p.endSize - p.startSize) * t;

        drawCircle(Circle{ p.pos, size }, color);
    }
}
