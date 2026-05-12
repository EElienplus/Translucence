//
// Created by Stěpán Toman on 04.05.2026.
//

#include "Input.hpp"
#include "Renderer.hpp"
#include "LayoutManager.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <unordered_map>

#include "Math.hpp"

namespace {
    // Per-widget animation state, keyed by widget pointer.
    // Stores eased values for hover/press transitions so visuals don't snap.
    struct WidgetAnim {
        float hover = 0.0f;
        float press = 0.0f;
        float focus = 0.0f;
        uint64_t lastTick = 0;
    };

    WidgetAnim& animFor(const void* id) {
        static std::unordered_map<const void*, WidgetAnim> table;
        return table[id];
    }

    // Frame-rate independent exponential smoothing toward a target value.
    // `rate` is roughly "convergence per second" — higher = snappier.
    float smoothTo(float current, float target, float dt, float rate) {
        float t = 1.0f - std::exp(-rate * dt);
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        return current + (target - current) * t;
    }

    SDL_Color deriveHover(SDL_Color base) {
        return Color::lighten(base, 0.10f);
    }
    SDL_Color deriveActive(SDL_Color base) {
        return Color::darken(base, 0.12f);
    }
}


void Renderer::clearBackground(SDL_Color color) {
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    SDL_RenderClear(app.getRenderer());
}
void Renderer::render() {
    SDL_RenderPresent(app.getRenderer());
    Input::endFrame();
    end();

}
void Renderer::drawRect(Rect rect, SDL_Color color) {
    SDL_FRect fRect = { rect.x, rect.y, rect.w, rect.h };
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(app.getRenderer(), &fRect);
}
void Renderer::drawRectOutline(Rect rect, SDL_Color color, int thickness) {
    float t = static_cast<float>(thickness);
    SDL_FColor fColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

    float halfT = t * 0.5f;
    float outerL = rect.x - halfT;
    float outerR = rect.x + rect.w + halfT;
    float outerT = rect.y - halfT;
    float outerB = rect.y + rect.h + halfT;

    float innerL = rect.x + halfT;
    float innerR = rect.x + rect.w - halfT;
    float innerT = rect.y + halfT;
    float innerB = rect.y + rect.h - halfT;

    std::vector<SDL_Vertex> vertices = {
        // Top
        {{outerL, outerT}, fColor, {0}}, {{outerR, outerT}, fColor, {0}}, {{innerL, innerT}, fColor, {0}},
        {{outerR, outerT}, fColor, {0}}, {{innerR, innerT}, fColor, {0}}, {{innerL, innerT}, fColor, {0}},
        // Right
        {{outerR, outerT}, fColor, {0}}, {{outerR, outerB}, fColor, {0}}, {{innerR, innerT}, fColor, {0}},
        {{outerR, outerB}, fColor, {0}}, {{innerR, innerB}, fColor, {0}}, {{innerR, innerT}, fColor, {0}},
        // Bottom
        {{outerR, outerB}, fColor, {0}}, {{outerL, outerB}, fColor, {0}}, {{innerR, innerB}, fColor, {0}},
        {{outerL, outerB}, fColor, {0}}, {{innerL, innerB}, fColor, {0}}, {{innerR, innerB}, fColor, {0}},
        // Left
        {{outerL, outerB}, fColor, {0}}, {{outerL, outerT}, fColor, {0}}, {{innerL, innerB}, fColor, {0}},
        {{outerL, outerT}, fColor, {0}}, {{innerL, innerT}, fColor, {0}}, {{innerL, innerB}, fColor, {0}}
    };

    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices.data(), (int)vertices.size(), nullptr, 0);
}
void Renderer::drawRoundedRect(Rect rect, SDL_Color color, float radius) {
    if (radius <= 0.0f) {
        drawRect(rect, color);
        return;
    }

    radius = std::min(radius, std::min(rect.w, rect.h) * 0.5f);
    SDL_FColor fColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
    std::vector<SDL_Vertex> vertices;

    auto addCorner = [&](float2 center, float startAngle) {
        const int segments = 16;
        float angleStep = (M_PI * 0.5f) / segments;
        for (int i = 0; i < segments; i++) {
            float a1 = startAngle + i * angleStep;
            float a2 = startAngle + (i + 1) * angleStep;

            vertices.push_back({ {center.x, center.y}, fColor, {0} });
            vertices.push_back({ {center.x + cosf(a1) * radius, center.y + sinf(a1) * radius}, fColor, {0} });
            vertices.push_back({ {center.x + cosf(a2) * radius, center.y + sinf(a2) * radius}, fColor, {0} });
        }
    };

    addCorner({ rect.x + radius, rect.y + radius }, M_PI);
    addCorner({ rect.x + rect.w - radius, rect.y + radius }, 1.5f * M_PI);
    addCorner({ rect.x + rect.w - radius, rect.y + rect.h - radius }, 0.0f);
    addCorner({ rect.x + radius, rect.y + rect.h - radius }, 0.5f * M_PI);

    drawRect({ rect.x + radius, rect.y, rect.w - 2 * radius, rect.h }, color);
    drawRect({ rect.x, rect.y + radius, radius, rect.h - 2 * radius }, color);
    drawRect({ rect.x + rect.w - radius, rect.y + radius, radius, rect.h - 2 * radius }, color);

    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices.data(), (int)vertices.size(), nullptr, 0);
}
void Renderer::drawRoundedRectOutline(Rect rect, SDL_Color color, float radius, int thickness) {
    if (radius <= 0.0f) {
        drawRectOutline(rect, color, thickness);
        return;
    }

    radius = std::min(radius, std::min(rect.w, rect.h) * 0.5f);
    float halfT = thickness * 0.5f;
    float innerR = radius - halfT;
    float outerR = radius + halfT;

    SDL_FColor fColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
    std::vector<SDL_Vertex> vertices;

    auto addCornerOutline = [&](float2 center, float startAngle) {
        const int segments = 16;
        float angleStep = (M_PI * 0.5f) / segments;
        for (int i = 0; i < segments; i++) {
            float a1 = startAngle + i * angleStep;
            float a2 = startAngle + (i + 1) * angleStep;

            float c1 = cosf(a1), s1 = sinf(a1);
            float c2 = cosf(a2), s2 = sinf(a2);

            vertices.push_back({ {center.x + innerR * c1, center.y + innerR * s1}, fColor, {0} });
            vertices.push_back({ {center.x + outerR * c1, center.y + outerR * s1}, fColor, {0} });
            vertices.push_back({ {center.x + innerR * c2, center.y + innerR * s2}, fColor, {0} });

            vertices.push_back({ {center.x + outerR * c1, center.y + outerR * s1}, fColor, {0} });
            vertices.push_back({ {center.x + outerR * c2, center.y + outerR * s2}, fColor, {0} });
            vertices.push_back({ {center.x + innerR * c2, center.y + innerR * s2}, fColor, {0} });
        }
    };

    // Corners
    addCornerOutline({ rect.x + radius, rect.y + radius }, M_PI);
    addCornerOutline({ rect.x + rect.w - radius, rect.y + radius }, 1.5f * M_PI);
    addCornerOutline({ rect.x + rect.w - radius, rect.y + rect.h - radius }, 0.0f);
    addCornerOutline({ rect.x + radius, rect.y + rect.h - radius }, 0.5f * M_PI);

    // Connecting Lines (Top, Right, Bottom, Left)
    drawLine({ rect.x + radius, rect.y }, { rect.x + rect.w - radius, rect.y }, color, thickness);
    drawLine({ rect.x + rect.w, rect.y + radius }, { rect.x + rect.w, rect.y + rect.h - radius }, color, thickness);
    drawLine({ rect.x + rect.w - radius, rect.y + rect.h }, { rect.x + radius, rect.y + rect.h }, color, thickness);
    drawLine({ rect.x, rect.y + rect.h - radius }, { rect.x, rect.y + radius }, color, thickness);

    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices.data(), (int)vertices.size(), nullptr, 0);
}
void Renderer::drawCircle(Circle circle, SDL_Color color) {
    const int segments = 64;
    std::vector<SDL_Vertex> vertices;
    float angleStep = 2.0f * M_PI / segments;
    SDL_FColor fColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

    for (int i = 0; i < segments; i++) {
        float a1 = i * angleStep;
        float a2 = (i + 1) * angleStep;

        vertices.push_back({ {circle.pos.x, circle.pos.y}, fColor, {0} });
        vertices.push_back({ {circle.pos.x + cosf(a1) * circle.radius, circle.pos.y + sinf(a1) * circle.radius}, fColor, {0} });
        vertices.push_back({ {circle.pos.x + cosf(a2) * circle.radius, circle.pos.y + sinf(a2) * circle.radius}, fColor, {0} });
    }
    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices.data(), (int)vertices.size(), nullptr, 0);
}
void Renderer::drawCircleOutline(Circle circle, SDL_Color color, int thickness) {
    const int segments = 64;
    float angleStep = 2.0f * M_PI / segments;
    float halfT = thickness * 0.5f;
    float innerR = circle.radius - halfT;
    float outerR = circle.radius + halfT;

    SDL_FColor fColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
    std::vector<SDL_Vertex> vertices;

    for (int i = 0; i < segments; i++) {
        float a1 = i * angleStep;
        float a2 = (i + 1) * angleStep;

        float c1 = cosf(a1), s1 = sinf(a1);
        float c2 = cosf(a2), s2 = sinf(a2);

        vertices.push_back({ {circle.pos.x + innerR * c1, circle.pos.y + innerR * s1}, fColor, {0} });
        vertices.push_back({ {circle.pos.x + outerR * c1, circle.pos.y + outerR * s1}, fColor, {0} });
        vertices.push_back({ {circle.pos.x + innerR * c2, circle.pos.y + innerR * s2}, fColor, {0} });

        vertices.push_back({ {circle.pos.x + outerR * c1, circle.pos.y + outerR * s1}, fColor, {0} });
        vertices.push_back({ {circle.pos.x + outerR * c2, circle.pos.y + outerR * s2}, fColor, {0} });
        vertices.push_back({ {circle.pos.x + innerR * c2, circle.pos.y + innerR * s2}, fColor, {0} });
    }
    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices.data(), (int)vertices.size(), nullptr, 0);
}
void Renderer::drawTriangle(Triangle triangle, SDL_Color color) {
    SDL_FColor fColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
    SDL_Vertex vertices[3] = {
        {{triangle.pointA.x, triangle.pointA.y}, fColor, {0}},
        {{triangle.pointB.x, triangle.pointB.y}, fColor, {0}},
        {{triangle.pointC.x, triangle.pointC.y}, fColor, {0}}
    };
    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices, 3, nullptr, 0);
}
void Renderer::drawTriangleOutline(Triangle triangle, SDL_Color color, int thickness) {
    drawLine(triangle.pointA, triangle.pointB, color, thickness);
    drawLine(triangle.pointB, triangle.pointC, color, thickness);
    drawLine(triangle.pointC, triangle.pointA, color, thickness);
}
void Renderer::drawShape(Shape shape, SDL_Color color) {
    if (shape.points.size() < 3) return;

    SDL_FColor fColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
    std::vector<SDL_Vertex> vertices;

    for (size_t i = 1; i < shape.points.size() - 1; i++) {
        vertices.push_back({ {shape.points[0].x, shape.points[0].y}, fColor, {0} });
        vertices.push_back({ {shape.points[i].x, shape.points[i].y}, fColor, {0} });
        vertices.push_back({ {shape.points[i+1].x, shape.points[i+1].y}, fColor, {0} });
    }

    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices.data(), (int)vertices.size(), nullptr, 0);
}
void Renderer::drawShapeOutline(Shape shape, SDL_Color color, int thickness) {
    if (shape.points.size() < 2) return;

    for (size_t i = 0; i < shape.points.size(); i++) {
        float2 start = shape.points[i];
        float2 end = shape.points[(i + 1) % shape.points.size()];
        drawLine(start, end, color, thickness);
    }
}
void Renderer::drawLine(float2 startPos, float2 endPos, SDL_Color color, int thickness) {
    float dx = endPos.x - startPos.x;
    float dy = endPos.y - startPos.y;
    float length = std::sqrt(dx * dx + dy * dy);

    if (length < 0.001f) return;

    float scale = (thickness * 0.5f) / length;
    float nx = -dy * scale;
    float ny = dx * scale;

    SDL_FColor fColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

    SDL_Vertex vertices[6] = {
        {{startPos.x + nx, startPos.y + ny}, fColor, {0}},
        {{endPos.x + nx, endPos.y + ny}, fColor, {0}},
        {{startPos.x - nx, startPos.y - ny}, fColor, {0}},
        {{endPos.x + nx, endPos.y + ny}, fColor, {0}},
        {{endPos.x - nx, endPos.y - ny}, fColor, {0}},
        {{startPos.x - nx, startPos.y - ny}, fColor, {0}}
    };

    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices, 6, nullptr, 0);
}
void Renderer::drawTail(Tail& tail, float2 point, SDL_Color color, int thickness, bool fadeThickness) {
    tail.points.push_back(point);
    if (tail.points.size() > (size_t)tail.maxLength) {
        tail.points.erase(tail.points.begin());
    }

    if (tail.points.size() < 2) return;

    for (size_t i = 0; i < tail.points.size() - 1; i++) {
        // Fade the tail based on age
        float t = (float)(i + 1) / (float)tail.points.size();
        
        // Quadratic fade-out for smoother look
        float alphaMult = t * t; 
        
        SDL_Color fadedColor = color;
        fadedColor.a = (Uint8)((float)color.a * alphaMult);

        int currentThickness = thickness;
        if (fadeThickness) {
            currentThickness = std::max(1, (int)((float)thickness * t));
        }
        
        drawLine(tail.points[i], tail.points[i + 1], fadedColor, currentThickness);
    }
}

void Renderer::drawBezier(float2 startPos, float2 endPos, std::vector<float2> controlPoints, SDL_Color color, int thickness) {
    // resolution + 1 vertices, 4 vertices per cross-section
    constexpr int resolution = 48; // Balanced for high perf and smoothness
    constexpr int totalVertices = (resolution + 1) * 4;
    constexpr int totalIndices = resolution * 18;
    const float AA_WIDTH = 1.2f;

    // Use stack-allocated arrays to avoid heap thrashing every frame
    SDL_Vertex vertices[totalVertices];
    static int indices[totalIndices];
    static bool indicesInitialized = false;

    if (!indicesInitialized) {
        for (int i = 0; i < resolution; i++) {
            int curr = i * 4;
            int next = (i + 1) * 4;
            int offset = i * 18;
            int quad[] = {
                curr+0, curr+1, next+1, curr+0, next+1, next+0,
                curr+1, curr+2, next+2, curr+1, next+2, next+1,
                curr+2, curr+3, next+3, curr+2, next+3, next+2
            };
            std::copy(std::begin(quad), std::end(quad), &indices[offset]);
        }
        indicesInitialized = true;
    }

    float halfThick = (thickness * 0.5f) - (AA_WIDTH * 0.5f);
    if (halfThick < 0.0f) halfThick = 0.0f;

    SDL_FColor coreColor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
    SDL_FColor edgeColor = { coreColor.r, coreColor.g, coreColor.b, 0.0f };

    // Temporary buffer for De Casteljau to avoid vector copies
    // Max 10 points (start, end + 8 controls) is plenty for most UI
    float2 pointsStack[10];
    int numPoints = std::min((int)(controlPoints.size() + 2), 10);
    pointsStack[0] = startPos;
    for(int i = 0; i < numPoints - 2; ++i) pointsStack[i+1] = controlPoints[i];
    pointsStack[numPoints - 1] = endPos;

    auto getBezierPoint = [&](float t) {
        float2 tmp[10];
        std::copy(std::begin(pointsStack), std::begin(pointsStack) + numPoints, std::begin(tmp));
        for (int k = 1; k < numPoints; k++) {
            for (int i = 0; i < numPoints - k; i++) {
                tmp[i].x = tmp[i].x + (tmp[i + 1].x - tmp[i].x) * t;
                tmp[i].y = tmp[i].y + (tmp[i + 1].y - tmp[i].y) * t;
            }
        }
        return tmp[0];
    };

    float2 prevP = startPos;
    for (int i = 0; i <= resolution; i++) {
        float t = i / (float)resolution;
        float2 p = getBezierPoint(t);
        float2 dir;

        // Efficient tangent calculation
        if (i < resolution) {
            float2 nextP = getBezierPoint((i + 1) / (float)resolution);
            dir = { nextP.x - p.x, nextP.y - p.y };
        } else {
            dir = { p.x - prevP.x, p.y - prevP.y };
        }

        float invLen = 1.0f / std::sqrt(dir.x * dir.x + dir.y * dir.y + 1e-6f);
        float2 normal = {-dir.y * invLen, dir.x * invLen};

        int vIdx = i * 4;
        vertices[vIdx + 0] = {{p.x + normal.x * (halfThick + AA_WIDTH), p.y + normal.y * (halfThick + AA_WIDTH)}, edgeColor, {0}};
        vertices[vIdx + 1] = {{p.x + normal.x * halfThick, p.y + normal.y * halfThick}, coreColor, {0}};
        vertices[vIdx + 2] = {{p.x - normal.x * halfThick, p.y - normal.y * halfThick}, coreColor, {0}};
        vertices[vIdx + 3] = {{p.x - normal.x * (halfThick + AA_WIDTH), p.y - normal.y * (halfThick + AA_WIDTH)}, edgeColor, {0}};
        prevP = p;
    }

    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices, totalVertices, indices, totalIndices);
}
void Renderer::drawText(std::string text, float2 pos, SDL_Color color, int textSize) {
    if (text.empty()) return;

    if (TTF_GetFontSize(app.getFont()) != (float)textSize) {
        TTF_SetFontSize(app.getFont(), (float)textSize);
    }

    TTF_Text* ttfText = TTF_CreateText(app.getTextEngine(), app.getFont(), text.c_str(), 0);

    if (ttfText) {
        TTF_SetTextColor(ttfText, color.r, color.g, color.b, color.a);
        TTF_UpdateText(ttfText);

        if (!TTF_DrawRendererText(ttfText, pos.x, pos.y)) {
            SDL_Log("Draw failed: %s", SDL_GetError());
        }

        TTF_DestroyText(ttfText);
    } else {
        SDL_Log("Text creation failed: %s", SDL_GetError());
    }
}
void Renderer::drawImage(const RawImage &image, float2 pos, float scale) {
    if (image.data.empty() || image.w <= 0 || image.h <= 0) return;

    float drawW = static_cast<float>(image.w) * scale;
    float drawH = static_cast<float>(image.h) * scale;

    int pitch = image.w * sizeof(SDL_Color);
    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        image.w,
        image.h,
        SDL_PIXELFORMAT_RGBA32,
        (void*)image.data.data(),
        pitch
    );

    if (!surface) {
        SDL_Log("Surface creation failed: %s", SDL_GetError());
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(app.getRenderer(), surface);

    if (texture) {
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        drawRect({pos.x, pos.y, drawW, drawH}, Color::White);

        SDL_FRect destRect = { pos.x, pos.y, drawW, drawH };
        SDL_RenderTexture(app.getRenderer(), texture, nullptr, &destRect);

        SDL_DestroyTexture(texture);
    } else {
        SDL_Log("Texture creation failed: %s", SDL_GetError());
    }

    SDL_DestroySurface(surface);
}

void Renderer::drawSprite(const Sprite &sprite, float scale) {
    if (!sprite.getTexture()) return;
    SDL_FRect destRect = { sprite.pos.x, sprite.pos.y, sprite.width * scale, sprite.height * scale};
    SDL_RenderTexture(app.getRenderer(), sprite.getTexture(), nullptr, &destRect);
}

void Renderer::drawList(const std::vector<std::string>& list, float2 pos, SDL_Color color, int textSize) {
    if (list.empty()) return;

    if (TTF_GetFontSize(app.getFont()) != (float)textSize) {
        TTF_SetFontSize(app.getFont(), (float)textSize);
    }

    int lineSkip = TTF_GetFontLineSkip(app.getFont());
    float currentY = pos.y;

    for (const std::string& item : list) {
        drawText(item, {pos.x, currentY}, color, textSize);
        currentY += static_cast<float>(lineSkip);
    }
}

Button& Renderer::drawButton(Button& params) {
    float2 mousePos = Input::getMousePos();
    params.isHovered = Input::isMouseHoveringRect(mousePos, params.rect);
    params.isClicked = params.isHovered && Input::isMouseClicked();
    params.isClickedOnce = params.isHovered && Input::isMouseButtonPressed(static_cast<uint8_t>(Input::MouseButton::LEFT));

    // Smooth state transitions
    WidgetAnim& anim = animFor(&params);
    uint64_t now = SDL_GetTicks();
    float dt = anim.lastTick == 0 ? 0.016f : static_cast<float>(now - anim.lastTick) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;
    anim.lastTick = now;
    anim.hover = smoothTo(anim.hover, params.isHovered ? 1.0f : 0.0f, dt, 18.0f);
    anim.press = smoothTo(anim.press, params.isClicked ? 1.0f : 0.0f, dt, 26.0f);

    // Derive hover/click colors automatically if the user left them Transparent
    SDL_Color baseBg     = params.bgColor;
    SDL_Color baseOutline= params.outlineColor;
    SDL_Color hoverBg    = Math::colorMatch(params.hoverColor, Color::Transparent)
                            ? deriveHover(baseBg) : params.hoverColor;
    SDL_Color clickBg    = Math::colorMatch(params.clickColor, Color::Transparent)
                            ? deriveActive(baseBg) : params.clickColor;
    SDL_Color hoverOl    = Math::colorMatch(params.hoverOutlineColor, Color::Transparent)
                            ? Color::lighten(baseOutline, 0.25f) : params.hoverOutlineColor;
    SDL_Color clickOl    = Math::colorMatch(params.clickOutlineColor, Color::Transparent)
                            ? Color::lighten(baseOutline, 0.40f) : params.clickOutlineColor;

    int baseOlW  = params.outlineWidth;
    int hoverOlW = params.hoverOutlineWidth < 0 ? baseOlW : params.hoverOutlineWidth;
    int clickOlW = params.clickOutlineWidth < 0 ? baseOlW : params.clickOutlineWidth;

    // Blend: bg first hover -> base, then press dominates
    SDL_Color bgWithHover = Color::mix(baseBg, hoverBg, anim.hover);
    SDL_Color currentBgColor = Color::mix(bgWithHover, clickBg, anim.press);

    SDL_Color olWithHover = Color::mix(baseOutline, hoverOl, anim.hover);
    SDL_Color currentOutlineColor = Color::mix(olWithHover, clickOl, anim.press);
    int currentOutlineWidth = static_cast<int>(
        Math::lerp(static_cast<float>(baseOlW),
                   Math::lerp(static_cast<float>(hoverOlW), static_cast<float>(clickOlW), anim.press),
                   anim.hover));

    // Subtle press: nudge the button down a couple px when clicked
    Rect rect = params.rect;
    rect.y += anim.press * 1.5f;

    // Drop shadow — soft, offset, fades on press
    if (!Math::colorMatch(baseBg, Color::Transparent)) {
        float shadowAlpha = (1.0f - anim.press * 0.5f) * 0.35f;
        SDL_Color shadow = { 0, 0, 0, static_cast<Uint8>(shadowAlpha * 255.0f) };
        Rect shadowRect = { rect.x, rect.y + 2.0f, rect.w, rect.h };
        if (params.roundRadius > 0.0f) {
            drawRoundedRect(shadowRect, shadow, static_cast<float>(params.roundRadius));
        } else {
            drawRect(shadowRect, shadow);
        }
    }

    if (params.roundRadius > 0.0f) {
        drawRoundedRect(rect, currentBgColor, static_cast<float>(params.roundRadius));
    } else {
        drawRect(rect, currentBgColor);
    }

    if (currentOutlineWidth > 0 && !Math::colorMatch(currentOutlineColor, Color::Transparent)) {
        if (params.roundRadius > 0.0f) {
            drawRoundedRectOutline(rect, currentOutlineColor, static_cast<float>(params.roundRadius), currentOutlineWidth);
        } else {
            drawRectOutline(rect, currentOutlineColor, currentOutlineWidth);
        }
    }

    // Subtle top-edge highlight for a touch of dimensionality
    if (params.roundRadius > 0 && rect.h > 4.0f) {
        SDL_Color hi = { 255, 255, 255, static_cast<Uint8>(28 + anim.hover * 22) };
        Rect highlight = {
            rect.x + static_cast<float>(params.roundRadius) * 0.6f,
            rect.y + 1.0f,
            rect.w - static_cast<float>(params.roundRadius) * 1.2f,
            1.0f
        };
        if (highlight.w > 0) drawRect(highlight, hi);
    }

    // Text rendering — use user-specified size when it fits, else auto-shrink
    int textSize = params.textSize;
    if (!params.text.empty()) {
        int maxPossibleSize = Math::getSizeWithinButton(app.getFont(), params.text, rect);
        if (textSize <= 0 || textSize > maxPossibleSize) {
            textSize = maxPossibleSize;
        }
        float2 textPos = Math::getPosWithinButton(app.getFont(), params.text, textSize, rect);
        drawText(params.text, textPos, params.textColor, textSize);
    }

    return params;
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

Button& Renderer::drawButton(Button& params, LayoutManager& layout, float w, float h) {
    params.rect = layout.next(w, h);
    return drawButton(params);
}

Button& Renderer::drawButton(Button& params, float w, float h) {
    if (activeLayout) params.rect = activeLayout->next(w, h);
    return drawButton(params);
}
Slider& Renderer::drawSlider(Slider& params) {
    static void* activeID = nullptr;

    float2 mousePos = Input::getMousePos();
    bool mouseHeld = Input::isMouseClicked();
    bool hovering  = Input::isMouseHoveringRect(mousePos, params.rect);

    if (mouseHeld && hovering && activeID == nullptr)
        activeID = static_cast<void*>(&params);
    if (!mouseHeld && activeID == static_cast<void*>(&params))
        activeID = nullptr;

    bool isActive = activeID == static_cast<void*>(&params);
    bool isVertical = params.rect.h > params.rect.w;

    if (isActive) {
        if (isVertical) {
            float localY = mousePos.y - params.rect.y;
            params.value = Math::clamp(0.0f, 1.0f, localY / params.rect.h);
        } else {
            float localX = mousePos.x - params.rect.x;
            params.value = Math::clamp(0.0f, 1.0f, localX / params.rect.w);
        }
    }

    float t = Math::clamp(0.0f, 1.0f, params.value);

    // Smooth knob hover/press animation
    WidgetAnim& anim = animFor(&params);
    uint64_t now = SDL_GetTicks();
    float dt = anim.lastTick == 0 ? 0.016f : static_cast<float>(now - anim.lastTick) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;
    anim.lastTick = now;
    anim.hover = smoothTo(anim.hover, hovering ? 1.0f : 0.0f, dt, 16.0f);
    anim.press = smoothTo(anim.press, isActive ? 1.0f : 0.0f, dt, 22.0f);

    // Track — thinner pill in the centre rather than the full rect, with rounded ends.
    const float trackThickness = std::min(
        isVertical ? params.rect.w : params.rect.h,
        8.0f + 4.0f * anim.hover
    );
    Rect track;
    if (isVertical) {
        track = {
            params.rect.x + (params.rect.w - trackThickness) * 0.5f,
            params.rect.y,
            trackThickness,
            params.rect.h
        };
    } else {
        track = {
            params.rect.x,
            params.rect.y + (params.rect.h - trackThickness) * 0.5f,
            params.rect.w,
            trackThickness
        };
    }
    float trackRadius = trackThickness * 0.5f;
    drawRoundedRect(track, params.color, trackRadius);

    // Filled portion in accent color
    SDL_Color fill = Math::colorMatch(params.fillColor, Color::Transparent)
                        ? Color::Accent : params.fillColor;
    Rect fillRect;
    if (isVertical) {
        fillRect = { track.x, track.y, track.w, track.h * t };
    } else {
        fillRect = { track.x, track.y, track.w * t, track.h };
    }
    if (fillRect.w > 0 && fillRect.h > 0) {
        drawRoundedRect(fillRect, fill, trackRadius);
    }

    // Knob position
    float knobX, knobY;
    if (isVertical) {
        knobX = params.rect.x + params.rect.w * 0.5f;
        knobY = Math::lerp(params.rect.y, params.rect.y + params.rect.h, t);
    } else {
        knobX = Math::lerp(params.rect.x, params.rect.x + params.rect.w, t);
        knobY = params.rect.y + params.rect.h * 0.5f;
    }

    float baseR = static_cast<float>(params.knobSize) * 0.5f;
    float knobRadius = baseR + anim.hover * 1.5f - anim.press * 0.6f;

    // Soft glow around knob on hover/active
    float glowAlpha = (anim.hover * 0.35f + anim.press * 0.25f);
    if (glowAlpha > 0.01f) {
        SDL_Color glow = { fill.r, fill.g, fill.b, static_cast<Uint8>(glowAlpha * 255.0f) };
        drawCircle({ {knobX, knobY}, knobRadius + 6.0f }, glow);
    }

    // Shadow under knob
    SDL_Color knobShadow = { 0, 0, 0, 90 };
    drawCircle({ {knobX, knobY + 1.5f}, knobRadius }, knobShadow);

    // Knob itself
    drawCircle({ {knobX, knobY}, knobRadius }, params.knobColor);
    // Outline ring for a crisper edge
    drawCircleOutline({ {knobX, knobY}, knobRadius }, Color::withAlpha(Color::Border, 180), 1);

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
    float2 mousePos = Input::getMousePos();
    bool hovering = Input::isMouseHoveringRect(mousePos, params.rect);

    // 1. Focus logic
    if (Input::isMouseClicked()) {
        if (hovering && !params.enabled) {
            params.enabled = true;
            SDL_StartTextInput(app.getWindow());
        } else if (!hovering && params.enabled) {
            params.enabled = false;
            SDL_StopTextInput(app.getWindow());
        }
    }

    // 2. Text input handling
    if (params.enabled) {
        const std::string& frameText = Input::getLastTextInput();
        if (!frameText.empty()) params.value += frameText;

        static uint64_t lastDeleteTime = 0;
        const uint64_t deleteDelay = 400;
        const uint64_t deleteInterval = 40;

        if (Input::isKeyDown(SDL_SCANCODE_BACKSPACE)) {
            uint64_t now = SDL_GetTicks();
            if (Input::isKeyPressed(SDL_SCANCODE_BACKSPACE)) {
                if (!params.value.empty()) params.value.pop_back();
                lastDeleteTime = now + deleteDelay;
            } else if (now > lastDeleteTime) {
                if (!params.value.empty()) params.value.pop_back();
                lastDeleteTime = now + deleteInterval;
            }
        }

        if (Input::isKeyPressed(SDL_SCANCODE_RETURN)) {
            params.value += '\n';
        }

        if (Input::isKeyPressed(SDL_SCANCODE_ESCAPE)) {
            params.enabled = false;
            SDL_StopTextInput(app.getWindow());
        }
    }

    // 3. Animation state for focus/hover transitions
    WidgetAnim& anim = animFor(&params);
    uint64_t now = SDL_GetTicks();
    float dt = anim.lastTick == 0 ? 0.016f : static_cast<float>(now - anim.lastTick) / 1000.0f;
    if (dt > 0.1f) dt = 0.1f;
    anim.lastTick = now;
    anim.hover = smoothTo(anim.hover, hovering ? 1.0f : 0.0f, dt, 16.0f);
    anim.focus = smoothTo(anim.focus, params.enabled ? 1.0f : 0.0f, dt, 18.0f);

    float radius = static_cast<float>(params.roundRadius);

    // 4. Soft focus glow — drawn outside the field
    if (anim.focus > 0.02f) {
        SDL_Color glow = Color::withAlpha(params.focusColor, static_cast<Uint8>(anim.focus * 70));
        Rect glowRect = {
            params.rect.x - 3.0f,
            params.rect.y - 3.0f,
            params.rect.w + 6.0f,
            params.rect.h + 6.0f
        };
        drawRoundedRect(glowRect, glow, radius + 3.0f);
    }

    // 5. Background
    SDL_Color bg = Color::mix(params.color, Color::lighten(params.color, 0.08f), anim.hover * 0.6f);
    drawRoundedRect(params.rect, bg, radius);

    // 6. Border — transitions to focusColor on focus
    SDL_Color border = Color::mix(params.borderColor, params.focusColor, anim.focus);
    int borderW = anim.focus > 0.5f ? 2 : 1;
    drawRoundedRectOutline(params.rect, border, radius, borderW);

    // 7. Text layout
    float hPad = 12.0f;
    float vPad = 10.0f;
    float2 textPos = { params.rect.x + hPad, params.rect.y + vPad };
    int wrapWidth = static_cast<int>(params.rect.w - (hPad * 2));
    if (wrapWidth < 1) wrapWidth = 1;

    if (TTF_GetFontSize(app.getFont()) != (float)params.textSize) {
        TTF_SetFontSize(app.getFont(), (float)params.textSize);
    }

    int cursorX = 0;
    int cursorY = 0;

    if (params.value.empty()) {
        // Placeholder
        if (!params.placeholder.empty()) {
            TTF_Text* ph = TTF_CreateText(app.getTextEngine(), app.getFont(), params.placeholder.c_str(), 0);
            if (ph) {
                TTF_SetTextWrapWidth(ph, wrapWidth);
                TTF_SetTextColor(ph,
                    params.placeholderColor.r,
                    params.placeholderColor.g,
                    params.placeholderColor.b,
                    params.placeholderColor.a);
                TTF_DrawRendererText(ph, textPos.x, textPos.y);
                TTF_DestroyText(ph);
            }
        }
    } else {
        TTF_Text* ttfText = TTF_CreateText(app.getTextEngine(), app.getFont(), params.value.c_str(), 0);
        if (ttfText) {
            TTF_SetTextWrapWidth(ttfText, wrapWidth);
            TTF_SetTextColor(ttfText, params.textColor.r, params.textColor.g, params.textColor.b, params.textColor.a);
            TTF_DrawRendererText(ttfText, textPos.x, textPos.y);

            int lastCharIdx = static_cast<int>(params.value.length()) - 1;
            while (lastCharIdx >= 0 && (params.value[lastCharIdx] == ' ' || params.value[lastCharIdx] == '\n')) {
                lastCharIdx--;
            }

            if (lastCharIdx >= 0) {
                TTF_SubString subString;
                if (TTF_GetTextSubString(ttfText, lastCharIdx + 1, &subString)) {
                    cursorX = subString.rect.x;
                    cursorY = subString.rect.y;
                }
            }

            int spaceW = 0;
            TTF_GetStringSize(app.getFont(), " ", 0, &spaceW, nullptr);

            for (size_t i = static_cast<size_t>(lastCharIdx + 1); i < params.value.length(); i++) {
                if (params.value[i] == '\n') {
                    cursorX = 0;
                    cursorY += params.textSize;
                } else if (params.value[i] == ' ') {
                    cursorX += spaceW;
                    if (cursorX > wrapWidth) {
                        cursorX = 0;
                        cursorY += params.textSize;
                    }
                }
            }

            TTF_DestroyText(ttfText);
        }
    }

    // 8. Blinking caret — uses focus color with a soft fade rather than hard blink
    if (params.enabled) {
        float phase = static_cast<float>((SDL_GetTicks() % 1000)) / 1000.0f;
        float blink = 0.5f + 0.5f * std::cos(phase * 2.0f * M_PI);
        SDL_Color caret = Color::withAlpha(params.focusColor, static_cast<Uint8>(80 + 175 * blink));
        Rect cursorRect = {
            textPos.x + static_cast<float>(cursorX) + 1.0f,
            textPos.y + static_cast<float>(cursorY) + 2.0f,
            2.0f,
            static_cast<float>(params.textSize) - 4.0f
        };
        drawRoundedRect(cursorRect, caret, 1.0f);
    }

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
