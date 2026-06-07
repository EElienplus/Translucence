#include "Renderer.hpp"
#include "Input.hpp"
#include "LayoutManager.hpp"
#include "ImageProcess.hpp"
#include <cstring>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <cctype>

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

    if (sceneTexture) SDL_DestroyTexture(sceneTexture);
    if (bloomTexture1) SDL_DestroyTexture(bloomTexture1);
    if (bloomTexture2) SDL_DestroyTexture(bloomTexture2);
}

void Renderer::clearBackground(SDL_Color color) {
    // Automatically apply screenshake state modifications right before drawing begins
    applyScreenShake(app.getDeltaTime());

    ensureBloomTextures();

    if (activeLayout && app.isAutoResizeEnabled()) {
        activeLayout->reset({ 0, 0, (float)app.getWidth(), (float)app.getHeight() });
    }

    if (sceneTexture) {
        SDL_SetRenderTarget(app.getRenderer(), sceneTexture);
    }

    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    SDL_RenderClear(app.getRenderer());
}

void Renderer::render() {
    // Auto-reset layout stack to avoid buildup if user misses end() calls
    while (activeLayout && !activeLayout->getStateStack().empty()) {
        activeLayout->pop();
    }
    end();

    if (sceneTexture) {
        SDL_SetRenderTarget(app.getRenderer(), nullptr);
        SDL_SetTextureBlendMode(sceneTexture, SDL_BLENDMODE_NONE);
        SDL_SetTextureAlphaMod(sceneTexture, 255);
        SDL_SetTextureColorMod(sceneTexture, 255, 255, 255);
        SDL_RenderTexture(app.getRenderer(), sceneTexture, nullptr, nullptr);
    }

    // Safety fallback reset before buffer presentation occurs
    resetScreenShake();

    SDL_RenderPresent(app.getRenderer());
}

void Renderer::drawRect(Rect rect, SDL_Color color) {
    if (app.isAutoResizeEnabled()) {
        if (rect.h == (float)app.getInitialHeight()) rect.h = (float)app.getHeight();
        if (rect.w == (float)app.getInitialWidth()) rect.w = (float)app.getWidth();
    }
    SDL_FRect fRect = { rect.x, rect.y, rect.w, rect.h };
    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(app.getRenderer(), &fRect);
}

void Renderer::drawRectOutline(Rect rect, SDL_Color color, int thickness) {
    if (app.isAutoResizeEnabled()) {
        if (rect.h == (float)app.getInitialHeight()) rect.h = (float)app.getHeight();
        if (rect.w == (float)app.getInitialWidth()) rect.w = (float)app.getWidth();
    }
    if (thickness <= 1) {
        SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
        SDL_FRect fRect = { rect.x, rect.y, rect.w, rect.h };
        SDL_RenderRect(app.getRenderer(), &fRect);
    } else {
        float2 p1 = { rect.x, rect.y };
        float2 p2 = { rect.x + rect.w, rect.y };
        float2 p3 = { rect.x + rect.w, rect.y + rect.h };
        float2 p4 = { rect.x, rect.y + rect.h };

        drawThickLine(p1, p2, color, (float)thickness);
        drawThickLine(p2, p3, color, (float)thickness);
        drawThickLine(p3, p4, color, (float)thickness);
        drawThickLine(p4, p1, color, (float)thickness);
    }
}

void Renderer::drawRoundedRect(Rect rect, SDL_Color color, float radius) {
    if (app.isAutoResizeEnabled()) {
        if (rect.h == (float)app.getInitialHeight()) rect.h = (float)app.getHeight();
        if (rect.w == (float)app.getInitialWidth()) rect.w = (float)app.getWidth();
    }
    if (radius <= 0) {
        drawRect(rect, color);
        return;
    }

    // Ensure radius doesn't exceed half-dimensions
    radius = std::min({radius, rect.w / 2.0f, rect.h / 2.0f});

    SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);

    // 4 Corner Circles
    drawCircle(Circle{{rect.x + radius, rect.y + radius}, radius}, color);
    drawCircle(Circle{{rect.x + rect.w - radius, rect.y + radius}, radius}, color);
    drawCircle(Circle{{rect.x + radius, rect.y + rect.h - radius}, radius}, color);
    drawCircle(Circle{{rect.x + rect.w - radius, rect.y + rect.h - radius}, radius}, color);

    // 4 corner circles are drawn with radius 'r' which fills up to cx+dx, cy+dy.
    // To ensure perfect coverage without gaps between rectangles and circles,
    // we use a slightly more robust filling strategy or just use float increments in drawCircle.
    // The current drawCircle already uses float dy and dx.
    
    // 3 Rectangles to fill the middle
    // Horizontal middle (full width)
    drawRect(Rect{rect.x, rect.y + radius, rect.w, rect.h - radius * 2.0f}, color);
    // Top middle (between corners)
    drawRect(Rect{rect.x + radius, rect.y, rect.w - radius * 2.0f, radius}, color);
    // Bottom middle (between corners)
    drawRect(Rect{rect.x + radius, rect.y + rect.h - radius, rect.w - radius * 2.0f, radius}, color);
}

void Renderer::drawRoundedRectOutline(Rect rect, SDL_Color color, float radius, int thickness) {
    if (app.isAutoResizeEnabled()) {
        if (rect.h == (float)app.getInitialHeight()) rect.h = (float)app.getHeight();
        if (rect.w == (float)app.getInitialWidth()) rect.w = (float)app.getWidth();
    }
    if (radius <= 0) {
        drawRectOutline(rect, color, thickness);
        return;
    }

    radius = std::min({radius, rect.w / 2.0f, rect.h / 2.0f});

    auto drawArcThick = [&](float2 center, float startAngle, float endAngle) {
        constexpr int segments = 16;
        float2 lastPos = {
            center.x + std::cos(startAngle) * radius,
            center.y + std::sin(startAngle) * radius
        };
        for (int i = 1; i <= segments; ++i) {
            float t = (float)i / (float)segments;
            float angle = startAngle + t * (endAngle - startAngle);
            float2 currentPos = {
                center.x + std::cos(angle) * radius,
                center.y + std::sin(angle) * radius
            };
            drawLine(lastPos, currentPos, color, thickness);
            lastPos = currentPos;
        }
    };

    float pi = 3.14159265f;
    drawArcThick({rect.x + radius, rect.y + radius}, pi, 1.5f * pi); // Top-left
    drawArcThick({rect.x + rect.w - radius, rect.y + radius}, 1.5f * pi, 2.0f * pi); // Top-right
    drawArcThick({rect.x + rect.w - radius, rect.y + rect.h - radius}, 0, 0.5f * pi); // Bottom-right
    drawArcThick({rect.x + radius, rect.y + rect.h - radius}, 0.5f * pi, pi); // Bottom-left

    // Connect with lines
    // Top
    drawLine({rect.x + radius, rect.y}, {rect.x + rect.w - radius, rect.y}, color, thickness);
    // Bottom
    drawLine({rect.x + radius, rect.y + rect.h}, {rect.x + rect.w - radius, rect.y + rect.h}, color, thickness);
    // Left
    drawLine({rect.x, rect.y + radius}, {rect.x, rect.y + rect.h - radius}, color, thickness);
    // Right
    drawLine({rect.x + rect.w, rect.y + radius}, {rect.x + rect.w, rect.y + rect.h - radius}, color, thickness);
}

void Renderer::drawCircle(Circle circle, SDL_Color color) {
    constexpr int segments = 64;
    std::vector<SDL_Vertex> vertices;
    SDL_FColor fcolor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

    vertices.push_back({ { circle.pos.x, circle.pos.y }, fcolor, { 0, 0 } });
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i * 2.0f * 3.14159265f / (float)segments;
        float x = circle.pos.x + std::cos(angle) * circle.radius;
        float y = circle.pos.y + std::sin(angle) * circle.radius;
        vertices.push_back({ { x, y }, fcolor, { 0, 0 } });
    }

    std::vector<int> indices;
    for (int i = 1; i <= segments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices.data(), (int)vertices.size(), indices.data(), (int)indices.size());
}

void Renderer::drawCircleOutline(Circle circle, SDL_Color color, int thickness) {
    constexpr int segments = 64;
    float2 lastPos = { circle.pos.x + circle.radius, circle.pos.y };

    for (int i = 1; i <= segments; ++i) {
        float angle = (float)i * 2.0f * 3.14159265f / (float)segments;
        float2 currentPos = {
            circle.pos.x + std::cos(angle) * circle.radius,
            circle.pos.y + std::sin(angle) * circle.radius
        };
        drawLine(lastPos, currentPos, color, thickness);
        lastPos = currentPos;
    }
}

void Renderer::drawTriangle(Triangle triangle, SDL_Color color) {
    SDL_Vertex vertices[3];
    SDL_FColor fcolor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

    vertices[0] = { { triangle.pointA.x, triangle.pointA.y }, fcolor, { 0, 0 } };
    vertices[1] = { { triangle.pointB.x, triangle.pointB.y }, fcolor, { 0, 0 } };
    vertices[2] = { { triangle.pointC.x, triangle.pointC.y }, fcolor, { 0, 0 } };

    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices, 3, nullptr, 0);
}

void Renderer::drawTriangleOutline(Triangle triangle, SDL_Color color, int thickness) {
    if (thickness <= 1) {
        SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
        SDL_RenderLine(app.getRenderer(), triangle.pointA.x, triangle.pointA.y, triangle.pointB.x, triangle.pointB.y);
        SDL_RenderLine(app.getRenderer(), triangle.pointB.x, triangle.pointB.y, triangle.pointC.x, triangle.pointC.y);
        SDL_RenderLine(app.getRenderer(), triangle.pointC.x, triangle.pointC.y, triangle.pointA.x, triangle.pointA.y);
    } else {
        drawThickLine(triangle.pointA, triangle.pointB, color, (float)thickness);
        drawThickLine(triangle.pointB, triangle.pointC, color, (float)thickness);
        drawThickLine(triangle.pointC, triangle.pointA, color, (float)thickness);
    }
}

void Renderer::drawShape(Shape shape, SDL_Color color) {
    if (shape.points.size() < 3) return;
    
    std::vector<SDL_Vertex> vertices;
    SDL_FColor fcolor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
    
    // Very simple fan triangulation (only works for convex shapes)
    for (size_t i = 0; i < shape.points.size(); ++i) {
        vertices.push_back({ { shape.points[i].x, shape.points[i].y }, fcolor, { 0, 0 } });
    }
    
    std::vector<int> indices;
    for (size_t i = 1; i < shape.points.size() - 1; ++i) {
        indices.push_back(0);
        indices.push_back(static_cast<int>(i));
        indices.push_back(static_cast<int>(i + 1));
    }
    
    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices.data(), (int)vertices.size(), indices.data(), (int)indices.size());
}

void Renderer::drawShapeOutline(Shape shape, SDL_Color color, int thickness) {
    if (shape.points.size() < 2) return;
    if (thickness <= 1) {
        drawShape(shape, color);
    } else {
        for (size_t i = 0; i < shape.points.size() - 1; ++i) {
            drawThickLine(shape.points[i], shape.points[i+1], color, (float)thickness);
        }
    }
}

void Renderer::drawLine(float2 startPos, float2 endPos, SDL_Color color, int thickness) {
    if (thickness <= 1) {
        SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
        SDL_RenderLine(app.getRenderer(), startPos.x, startPos.y, endPos.x, endPos.y);
    } else {
        drawThickLine(startPos, endPos, color, (float)thickness);
    }
}
void Renderer::drawLine(Line line, SDL_Color color) {
    drawLine(line.startPos, line.endPos, color, line.thickness);
}

void Renderer::drawTail(Tail& tail, float2 point, SDL_Color color, int thickness, bool fadeThickness) {
    if (tail.points.empty() || Math::distance(tail.points.back(), point) > 0.1f) {
        tail.points.push_back(point);
    }
    
    while (tail.points.size() > static_cast<size_t>(tail.maxLength)) {
        tail.points.erase(tail.points.begin());
    }
    if (tail.points.size() < 2) return;

    for (size_t i = 0; i < tail.points.size() - 1; ++i) {
        float currentThickness = (float)thickness;
        if (fadeThickness) {
            currentThickness = (float)thickness * ((float)(i + 1) / (float)tail.points.size());
        }

        if (currentThickness <= 1.0f) {
            SDL_SetRenderDrawColor(app.getRenderer(), color.r, color.g, color.b, color.a);
            SDL_RenderLine(app.getRenderer(), tail.points[i].x, tail.points[i].y, tail.points[i+1].x, tail.points[i+1].y);
        } else {
            drawThickLine(tail.points[i], tail.points[i+1], color, currentThickness);
        }
    }
}

void Renderer::drawBezier(float2 startPos, float2 endPos, std::vector<float2> controlPoints, SDL_Color color, int thickness) {
    // Basic implementation for now, drawing segments
    const int segments = 20;
    float2 lastPos = startPos;
    
    auto getBezierPoint = [&](float t) {
        if (controlPoints.empty()) {
            return Math::lerpFloat2(startPos, endPos, t);
        } else if (controlPoints.size() == 1) {
            // Quadratic
            float2 p0 = startPos;
            float2 p1 = controlPoints[0];
            float2 p2 = endPos;
            float u = 1.0f - t;
            float tt = t * t;
            float uu = u * u;
            float2 p = Math::multiply(p0, uu);
            p = Math::add(p, Math::multiply(p1, 2 * u * t));
            p = Math::add(p, Math::multiply(p2, tt));
            return p;
        } else {
            // Simple linear for more control points for now
            return Math::lerpFloat2(startPos, endPos, t);
        }
    };

    for (int i = 1; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        float2 currentPos = getBezierPoint(t);
        drawLine(lastPos, currentPos, color, thickness);
        lastPos = currentPos;
    }
}

void Renderer::drawThickLine(float2 start, float2 end, SDL_Color color, float thickness) {
    float2 dir = Math::subtract(end, start);
    float2 norm = Math::normalize(dir);
    float2 perp = Math::perpendicular(norm);
    float2 offset = Math::multiply(perp, thickness / 2.0f);

    float2 p1 = Math::subtract(start, offset);
    float2 p2 = Math::add(start, offset);
    float2 p3 = Math::add(end, offset);
    float2 p4 = Math::subtract(end, offset);

    SDL_Vertex vertices[4];
    SDL_FColor fcolor = { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };

    vertices[0] = { { p1.x, p1.y }, fcolor, { 0, 0 } };
    vertices[1] = { { p2.x, p2.y }, fcolor, { 0, 0 } };
    vertices[2] = { { p3.x, p3.y }, fcolor, { 0, 0 } };
    vertices[3] = { { p4.x, p4.y }, fcolor, { 0, 0 } };

    int indices[6] = { 0, 1, 2, 2, 3, 0 };

    SDL_RenderGeometry(app.getRenderer(), nullptr, vertices, 4, indices, 6);
}

void Renderer::drawText(std::string text, float2 pos, SDL_Color color, int textSize) {
    if (text.empty()) return;
    if (!app.getFont() || !app.getTextEngine()) return;

    TTF_Font* font = app.getFont();
    if (TTF_GetFontSize(font) != static_cast<float>(textSize)) {
        TTF_SetFontSize(font, static_cast<float>(textSize));
    }

    TTF_Text* ttfText = TTF_CreateText(app.getTextEngine(), font, text.c_str(), 0);
    if (ttfText) {
        TTF_SetTextColor(ttfText, color.r, color.g, color.b, color.a);
        TTF_DrawRendererText(ttfText, pos.x, pos.y);
        TTF_DestroyText(ttfText);
    }
}

void Renderer::drawText(std::string text, SDL_Color color, int textSize, float w, float h) {
    if (text.empty()) return;
    if (!app.getFont() || !app.getTextEngine()) return;

    TTF_Font* font = app.getFont();
    float2 size = getTextSize(font, text, textSize);

    // If w or h is <= 0, we use the text's natural size
    float targetW = (w <= 0) ? size.x : w;
    float targetH = (h <= 0) ? size.y + 4 : h; // Added 4px vertical padding

    Rect r = getNextRect(targetW, targetH);

    // Center text within the allocated rectangle
    float2 pos = { r.x + (r.w - size.x) / 2.0f, r.y + (r.h - size.y) / 2.0f };
    drawText(text, pos, color, textSize);
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

Rect Renderer::getNextRect(float w, float h) {
    float finalW = w;
    float finalH = h;
    if (app.isAutoResizeEnabled()) {
        if (h == (float)app.getInitialHeight()) finalH = (float)app.getHeight();
        if (w == (float)app.getInitialWidth()) finalW = (float)app.getWidth();
    }
    if (activeLayout) return activeLayout->next(finalW, finalH);
    return { 0, 0, finalW, finalH };
}

Rect applyMagicScaling(const Application& app, Rect rect) {
    if (!app.isAutoResizeEnabled()) return rect;

    int iw = app.getInitialWidth();
    int ih = app.getInitialHeight();
    if (iw <= 0 || ih <= 0) return rect;

    int cw = app.getWidth();
    int ch = app.getHeight();

    Rect scaled = rect;
    if (static_cast<int>(rect.w) == iw) scaled.w = static_cast<float>(cw);
    if (static_cast<int>(rect.h) == ih) scaled.h = static_cast<float>(ch);
    if (static_cast<int>(rect.x) == iw) scaled.x = static_cast<float>(cw);
    if (static_cast<int>(rect.y) == ih) scaled.y = static_cast<float>(ch);

    return scaled;
}

void Renderer::row(float h, float padding, float spacing) {
    float finalH = h;
    if (app.isAutoResizeEnabled() && h == (float)app.getInitialHeight()) finalH = (float)app.getHeight();
    if (activeLayout) activeLayout->row(finalH, padding, spacing);
}

void Renderer::column(float w, float padding, float spacing) {
    float finalW = w;
    if (app.isAutoResizeEnabled() && w == (float)app.getInitialWidth()) finalW = (float)app.getWidth();
    if (activeLayout) activeLayout->column(finalW, padding, spacing);
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

void Renderer::drawButton(Button& params) {
    // Interactivity
    float2 mousePos = Input::getMousePos();
    params.isHovered = Input::isMouseHoveringRect(mousePos, params.rect);
    params.isClicked = params.isHovered && Input::isMouseButtonDown(static_cast<uint8_t>(Input::MouseButton::LEFT));
    params.isClickedOnce = params.isHovered && Input::isMouseButtonPressed(static_cast<uint8_t>(Input::MouseButton::LEFT));

    // Animations (Update state)
    params.update(app.getDeltaTime());

    // Get effective visual properties from component
    SDL_Color finalBg = params.getEffectiveBgColor();
    SDL_Color finalOutColor = params.getEffectiveOutlineColor();
    int outWidth = params.getEffectiveOutlineWidth();
    Rect drawRect = params.getAnimatedRect();

    if (params.roundRadius > 0) {
        drawRoundedRect(drawRect, finalBg, (float)params.roundRadius);
        drawRoundedRectOutline(drawRect, finalOutColor, (float)params.roundRadius, outWidth);
    } else {
        this->drawRect(drawRect, finalBg);
        drawRectOutline(drawRect, finalOutColor, outWidth);
    }

    if (!params.text.empty()) {
        int fontSize = Math::getSizeWithinButton(app.getFont(), params.text, params.rect);
        if (fontSize > params.textSize) fontSize = params.textSize;
        float2 textPos = Math::getPosWithinButton(app.getFont(), params.text, fontSize, drawRect);
        drawText(params.text, textPos, params.textColor, fontSize);
    }
}

void Renderer::drawButton(Button& params, LayoutManager& layout, float w, float h) {
    params.rect = layout.next(w, h);
    return drawButton(params);
}

void Renderer::drawButton(Button& params, float w, float h) {
    params.rect = getNextRect(w, h);
    return drawButton(params);
}

void Renderer::drawSlider(Slider& params) {
    float dt = app.getDeltaTime();
    float2 mousePos = Input::getMousePos();
    bool isHovered = Input::isMouseHoveringRect(mousePos, params.rect);

    if (params.enabled) {
        if (Input::isMouseButtonPressed(static_cast<uint8_t>(Input::MouseButton::LEFT))) {
            if (isHovered) {
                params.isDragging = true;
            }
        }

        if (params.isDragging) {
            if (Input::isMouseButtonDown(static_cast<uint8_t>(Input::MouseButton::LEFT))) {
                if (params.rect.w > 0) {
                    float newValue = (mousePos.x - params.rect.x) / params.rect.w;
                    newValue = std::max(0.0f, std::min(1.0f, newValue));
                    
                    if (params.step > 0.0f) {
                        newValue = std::round(newValue / params.step) * params.step;
                        newValue = std::max(0.0f, std::min(1.0f, newValue));
                    }
                    
                    params.value = newValue;
                }
            } else {
                params.isDragging = false;
            }
        }
    } else {
        params.isDragging = false;
    }

    params.update(dt, isHovered);

    // Visual styles
    SDL_Color trackColor = params.color;
    SDL_Color fillCol = params.fillColor;
    SDL_Color knobCol = params.knobColor;
    SDL_Color knobOutlineCol = params.knobOutlineColor;

    if (!params.enabled) {
        trackColor = Color::mix(trackColor, Color::Black, 0.2f);
        fillCol = Color::mix(fillCol, Color::Black, 0.4f);
        knobCol = Color::mix(knobCol, Color::Black, 0.2f);
    } else if (params.hoverProgress > 0.0f) {
        trackColor = Color::mix(trackColor, Color::lighten(trackColor, 0.05f), params.hoverProgress);
        fillCol = Color::mix(fillCol, Color::lighten(fillCol, 0.1f), params.hoverProgress);
    }

    // Draw track
    drawRoundedRect(params.rect, trackColor, (float)params.roundRadius);

    // Draw fill
    Rect fillRect = { params.rect.x, params.rect.y, params.rect.w * params.value, params.rect.h };
    drawRoundedRect(fillRect, fillCol, (float)params.roundRadius);

    // Draw knob
    float knobX = params.rect.x + (params.rect.w * params.value);
    float animatedKnobSize = static_cast<float>(params.knobSize);
    if (params.enabled) {
        animatedKnobSize += (params.hoverProgress * 4.0f);
    }
    
    Circle knobCircle = { {knobX, params.rect.y + params.rect.h * 0.5f}, animatedKnobSize * 0.5f };
    drawCircle(knobCircle, knobCol);
    
    // Knob outline
    if (params.enabled) {
         SDL_Color outlineCol = Color::mix(knobOutlineCol, Color::Accent, params.hoverProgress);
         drawCircleOutline(knobCircle, outlineCol, 1);
    } else {
         drawCircleOutline(knobCircle, knobOutlineCol, 1);
    }
    
}

void Renderer::drawSlider(Slider& params, LayoutManager& layout, float w, float h) {
    params.rect = layout.next(w, h);
    return drawSlider(params);
}

void Renderer::drawSlider(Slider& params, float w, float h) {
    params.rect = getNextRect(w, h);
    return drawSlider(params);
}

void Renderer::drawInputField(InputField& params) {
    float dt = app.getDeltaTime();
    TTF_Font* font = app.getFont();
    if (!font) return;

    bool isMousePressed = Input::isMouseButtonPressed(static_cast<uint8_t>(Input::MouseButton::LEFT));
    bool isMouseDown = Input::isMouseButtonDown(static_cast<uint8_t>(Input::MouseButton::LEFT));
    bool isHovered = Input::isMouseHoveringRect(Input::getMousePos(), params.rect);

    // Focus management
    if (isMousePressed) {
        if (isHovered) {
            if (focusedField != &params) {
                focusedField = &params;
                params.enabled = true;
                SDL_StartTextInput(app.getWindow());
                params.hasStartedTextInput = true;
                params.cursorPos = (int)params.value.length();
                params.clearSelection();
                return; // Skip further processing this frame to avoid immediate selection
            }
            
            // Set cursor position by click
            float margin = 5.0f;
            float lineSpacing = 4.0f;
            float lineHeight = (float)params.textSize + lineSpacing;
            
            if (params.multiLine) {
                float mouseRelY = Input::getMouseY() - (params.rect.y + 4.0f - params.verticalScrollOffset);
                int lineIdx = std::max(0, (int)std::floor(mouseRelY / lineHeight));
                
                std::vector<std::string> lines;
                std::string line;
                std::stringstream ss(params.value);
                while (std::getline(ss, line, '\n')) lines.push_back(line);
                if (!params.value.empty() && params.value.back() == '\n') lines.push_back("");
                if (params.value.empty()) lines.push_back("");
                
                if (lineIdx >= (int)lines.size()) lineIdx = (int)lines.size() - 1;
                
                int charOffset = 0;
                for (int i = 0; i < lineIdx; ++i) charOffset += (int)lines[i].length() + 1;
                
                float mouseRelX = Input::getMouseX() - (params.rect.x + margin);
                int bestPosInLine = 0;
                float bestDist = 10000.0f;
                const std::string& currentLineText = lines[lineIdx];
                for (int i = 0; i <= (int)currentLineText.length(); ++i) {
                    std::string sub = currentLineText.substr(0, i);
                    float2 subSize = getTextSize(font, sub, params.textSize);
                    float dist = std::abs(subSize.x - mouseRelX);
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestPosInLine = i;
                    }
                }
                params.cursorPos = charOffset + bestPosInLine;
            } else {
                float mouseRelX = Input::getMouseX() - (params.rect.x + margin - params.scrollOffset);
                int bestPos = 0;
                float bestDist = 10000.0f;
                for (int i = 0; i <= (int)params.value.length(); ++i) {
                    std::string sub = params.value.substr(0, i);
                    float2 subSize = getTextSize(font, sub, params.textSize);
                    float dist = std::abs(subSize.x - mouseRelX);
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestPos = i;
                    }
                }
                params.cursorPos = bestPos;
            }
            if (params.cursorPos < params.protectedLen) params.cursorPos = params.protectedLen;
            params.cursorVisible = true;
            params.cursorTimer = 0.0f;
            params.clearSelection();
        } else if (focusedField == &params) {
            focusedField = nullptr;
            params.enabled = false;
            SDL_StopTextInput(app.getWindow());
            params.hasStartedTextInput = false;
        }
    }

    // Support for manual enabled set
    if (params.enabled && focusedField != &params) {
        focusedField = &params;
        if (!params.hasStartedTextInput) {
            SDL_StartTextInput(app.getWindow());
            params.hasStartedTextInput = true;
        }
    }

    if (focusedField == &params) {
        float margin = 5.0f;
        float lineSpacing = 4.0f;
        float lineHeight = (float)params.textSize + lineSpacing;

        // Selection dragging
        if (isMouseDown && isHovered && !isMousePressed) {
            int targetPos = params.cursorPos;
            if (params.multiLine) {
                float mouseRelY = Input::getMouseY() - (params.rect.y + 4.0f - params.verticalScrollOffset);
                int lineIdx = std::max(0, (int)std::floor(mouseRelY / lineHeight));
                
                std::vector<std::string> lines;
                std::string line;
                std::stringstream ss(params.value);
                while (std::getline(ss, line, '\n')) lines.push_back(line);
                if (!params.value.empty() && params.value.back() == '\n') lines.push_back("");
                if (params.value.empty()) lines.push_back("");
                
                if (lineIdx >= (int)lines.size()) lineIdx = (int)lines.size() - 1;
                
                int charOffset = 0;
                for (int i = 0; i < lineIdx; ++i) charOffset += (int)lines[i].length() + 1;
                
                float mouseRelX = Input::getMouseX() - (params.rect.x + margin);
                int bestPosInLine = 0;
                float bestDist = 10000.0f;
                const std::string& currentLineText = lines[lineIdx];
                for (int i = 0; i <= (int)currentLineText.length(); ++i) {
                    std::string sub = currentLineText.substr(0, i);
                    float2 subSize = getTextSize(font, sub, params.textSize);
                    float dist = std::abs(subSize.x - mouseRelX);
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestPosInLine = i;
                    }
                }
                targetPos = charOffset + bestPosInLine;
            } else {
                float mouseRelX = Input::getMouseX() - (params.rect.x + margin - params.scrollOffset);
                int bestPos = 0;
                float bestDist = 10000.0f;
                for (int i = 0; i <= (int)params.value.length(); ++i) {
                    std::string sub = params.value.substr(0, i);
                    float2 subSize = getTextSize(font, sub, params.textSize);
                    float dist = std::abs(subSize.x - mouseRelX);
                    if (dist < bestDist) {
                        bestDist = dist;
                        bestPos = i;
                    }
                }
                targetPos = bestPos;
            }
            if (params.cursorPos != targetPos) {
                if (params.selectionStart == -1) params.selectionStart = params.cursorPos;
                params.cursorPos = targetPos;
                if (params.cursorPos < params.protectedLen) params.cursorPos = params.protectedLen;
            }
        }

        // Text Input
        const std::string& input = Input::getLastTextInput();
        if (!input.empty()) {
            if (params.hasSelection()) {
                auto [start, end] = params.getSelectionRange();
                params.value.erase(start, end - start);
                params.cursorPos = start;
                params.clearSelection();
            }
            
            // Filter
            std::string filteredInput;
            for (char c : input) {
                if (params.numericOnly && !std::isdigit(c) && c != '.' && c != '-') continue;
                if (!params.allowedChars.empty() && params.allowedChars.find(c) == std::string::npos) continue;
                filteredInput += c;
            }

            if (!filteredInput.empty()) {
                params.value.insert(params.cursorPos, filteredInput);
                params.cursorPos += (int)filteredInput.length();
                params.cursorVisible = true;
                params.cursorTimer = 0.0f;
                params.clearSelection(); // Ensure selection is cleared after typing
            }
            Input::clearTextInput();
        }

        bool ctrl = Input::isKeyDown(Input::Key::LCTRL) || Input::isKeyDown(Input::Key::RCTRL) || Input::isKeyDown(Input::Key::LGUI) || Input::isKeyDown(Input::Key::RGUI);
        bool shift = Input::isKeyDown(Input::Key::LSHIFT) || Input::isKeyDown(Input::Key::RSHIFT);

        auto handleKey = [&](Input::Key key, auto func) {
            if (Input::isKeyPressed(key)) {
                if (shift && params.selectionStart == -1) params.selectionStart = params.cursorPos;
                else if (!shift && key != Input::Key::BACKSPACE && key != Input::Key::DELETE) params.clearSelection();
                func();
                params.cursorVisible = true;
                params.cursorTimer = 0.0f;
            }
        };

        // Clipboard
        if (ctrl && Input::isKeyPressed(Input::Key::C)) {
            if (params.hasSelection()) {
                auto [start, end] = params.getSelectionRange();
                SDL_SetClipboardText(params.value.substr(start, end - start).c_str());
            }
        }
        if (ctrl && Input::isKeyPressed(Input::Key::X)) {
            if (params.hasSelection()) {
                auto [start, end] = params.getSelectionRange();
                SDL_SetClipboardText(params.value.substr(start, end - start).c_str());
                params.value.erase(start, end - start);
                params.cursorPos = start;
                params.clearSelection();
            }
        }
        if (ctrl && Input::isKeyPressed(Input::Key::V)) {
            char* text = SDL_GetClipboardText();
            if (text) {
                if (params.hasSelection()) {
                    auto [start, end] = params.getSelectionRange();
                    params.value.erase(start, end - start);
                    params.cursorPos = start;
                    params.clearSelection();
                }
                std::string s(text);
                params.value.insert(params.cursorPos, s);
                params.cursorPos += (int)s.length();
                SDL_free(text);
            }
        }
        if (ctrl && Input::isKeyPressed(Input::Key::A)) {
            params.selectionStart = 0;
            params.cursorPos = (int)params.value.length();
        }

        // Navigation & Editing
        handleKey(Input::Key::BACKSPACE, [&]() {
            if (params.hasSelection()) {
                auto [start, end] = params.getSelectionRange();
                int finalStart = std::max(start, params.protectedLen);
                if (finalStart < end) {
                    params.value.erase(finalStart, end - finalStart);
                    params.cursorPos = finalStart;
                }
                params.clearSelection();
            } else if (params.cursorPos > params.protectedLen) {
                if (ctrl) {
                    int oldPos = params.cursorPos;
                    // Jump to beginning of word, but not past protectedLen
                    while (params.cursorPos > params.protectedLen && std::isspace(params.value[params.cursorPos-1])) params.cursorPos--;
                    while (params.cursorPos > params.protectedLen && !std::isspace(params.value[params.cursorPos-1])) params.cursorPos--;
                    params.value.erase(params.cursorPos, oldPos - params.cursorPos);
                } else {
                    params.value.erase(params.cursorPos - 1, 1);
                    params.cursorPos--;
                }
            }
        });

        handleKey(Input::Key::DELETE, [&]() {
            if (params.hasSelection()) {
                auto [start, end] = params.getSelectionRange();
                int finalStart = std::max(start, params.protectedLen);
                if (finalStart < end) {
                    params.value.erase(finalStart, end - finalStart);
                    params.cursorPos = finalStart;
                }
                params.clearSelection();
            } else if (params.cursorPos < (int)params.value.length() && params.cursorPos >= params.protectedLen) {
                if (ctrl) {
                    int start = params.cursorPos;
                    while (params.cursorPos < (int)params.value.length() && std::isspace(params.value[params.cursorPos])) params.cursorPos++;
                    while (params.cursorPos < (int)params.value.length() && !std::isspace(params.value[params.cursorPos])) params.cursorPos++;
                    params.value.erase(start, params.cursorPos - start);
                    params.cursorPos = start;
                } else {
                    params.value.erase(params.cursorPos, 1);
                }
            }
        });

        handleKey(Input::Key::LEFT, [&]() {
            if (ctrl) {
                while (params.cursorPos > params.protectedLen && std::isspace(params.value[params.cursorPos-1])) params.cursorPos--;
                while (params.cursorPos > params.protectedLen && !std::isspace(params.value[params.cursorPos-1])) params.cursorPos--;
            } else {
                if (params.cursorPos > params.protectedLen) params.cursorPos--;
            }
        });

        handleKey(Input::Key::RIGHT, [&]() {
            if (ctrl) {
                while (params.cursorPos < (int)params.value.length() && std::isspace(params.value[params.cursorPos])) params.cursorPos++;
                while (params.cursorPos < (int)params.value.length() && !std::isspace(params.value[params.cursorPos])) params.cursorPos++;
            } else {
                if (params.cursorPos < (int)params.value.length()) params.cursorPos++;
            }
        });

        handleKey(Input::Key::UP, [&]() {
            if (params.multiLine) {
                std::vector<std::string> lines;
                std::string line;
                std::stringstream ss(params.value);
                while (std::getline(ss, line, '\n')) lines.push_back(line);
                if (!params.value.empty() && params.value.back() == '\n') lines.push_back("");
                if (params.value.empty()) lines.push_back("");

                int currentLineIdx = 0;
                int currentPosInLine = 0;
                int charCount = 0;
                for (int i = 0; i < (int)lines.size(); ++i) {
                    if (params.cursorPos >= charCount && params.cursorPos <= charCount + (int)lines[i].length()) {
                        currentLineIdx = i;
                        currentPosInLine = params.cursorPos - charCount;
                        break;
                    }
                    charCount += (int)lines[i].length() + 1;
                }

                if (currentLineIdx > 0) {
                    float currentX = getTextSize(font, lines[currentLineIdx].substr(0, currentPosInLine), params.textSize).x;
                    int prevLineIdx = currentLineIdx - 1;
                    int prevLineCharOffset = 0;
                    for (int i = 0; i < prevLineIdx; ++i) prevLineCharOffset += (int)lines[i].length() + 1;

                    int bestPos = 0;
                    float bestDist = 10000.0f;
                    for (int i = 0; i <= (int)lines[prevLineIdx].length(); ++i) {
                        float x = getTextSize(font, lines[prevLineIdx].substr(0, i), params.textSize).x;
                        float dist = std::abs(x - currentX);
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestPos = i;
                        }
                    }
                    params.cursorPos = prevLineCharOffset + bestPos;
                } else {
                    params.cursorPos = 0;
                }
            }
            if (params.cursorPos < params.protectedLen) params.cursorPos = params.protectedLen;
        });

        handleKey(Input::Key::DOWN, [&]() {
            if (params.multiLine) {
                std::vector<std::string> lines;
                std::string line;
                std::stringstream ss(params.value);
                while (std::getline(ss, line, '\n')) lines.push_back(line);
                if (!params.value.empty() && params.value.back() == '\n') lines.push_back("");
                if (params.value.empty()) lines.push_back("");

                int currentLineIdx = 0;
                int currentPosInLine = 0;
                int charCount = 0;
                for (int i = 0; i < (int)lines.size(); ++i) {
                    if (params.cursorPos >= charCount && params.cursorPos <= charCount + (int)lines[i].length()) {
                        currentLineIdx = i;
                        currentPosInLine = params.cursorPos - charCount;
                        break;
                    }
                    charCount += (int)lines[i].length() + 1;
                }

                if (currentLineIdx < (int)lines.size() - 1) {
                    float currentX = getTextSize(font, lines[currentLineIdx].substr(0, currentPosInLine), params.textSize).x;
                    int nextLineIdx = currentLineIdx + 1;
                    int nextLineCharOffset = 0;
                    for (int i = 0; i < nextLineIdx; ++i) nextLineCharOffset += (int)lines[i].length() + 1;

                    int bestPos = 0;
                    float bestDist = 10000.0f;
                    for (int i = 0; i <= (int)lines[nextLineIdx].length(); ++i) {
                        float x = getTextSize(font, lines[nextLineIdx].substr(0, i), params.textSize).x;
                        float dist = std::abs(x - currentX);
                        if (dist < bestDist) {
                            bestDist = dist;
                            bestPos = i;
                        }
                    }
                    params.cursorPos = nextLineCharOffset + bestPos;
                } else {
                    params.cursorPos = (int)params.value.length();
                }
            }
        });

        handleKey(Input::Key::HOME, [&]() { params.cursorPos = params.protectedLen; });
        handleKey(Input::Key::END, [&]() { params.cursorPos = (int)params.value.length(); });

        if (Input::isKeyPressed(Input::Key::ENTER)) {
            if (params.onTextSubmit) {
                std::string submitted = params.value.substr(params.protectedLen);
                params.onTextSubmit(submitted);
            } else if (params.multiLine) {
                if (params.hasSelection()) {
                    auto [start, end] = params.getSelectionRange();
                    int finalStart = std::max(start, params.protectedLen);
                    if (finalStart < end) {
                        params.value.erase(finalStart, end - finalStart);
                        params.cursorPos = finalStart;
                    }
                    params.clearSelection();
                }
                params.value.insert(params.cursorPos, "\n");
                params.cursorPos++;
                params.cursorVisible = true;
                params.cursorTimer = 0.0f;
            }
        }
    }

    params.update(dt);

    drawRoundedRect(params.rect, params.color, (float)params.roundRadius);
    drawRoundedRectOutline(params.rect, params.getEffectiveBorderColor(), (float)params.roundRadius, 1);

    // Set clipping
    SDL_Rect clipRect = { (int)params.rect.x, (int)params.rect.y, (int)params.rect.w, (int)params.rect.h };
    SDL_SetRenderClipRect(app.getRenderer(), &clipRect);

    float margin = 5.0f;
    std::string displayText = params.value;
    SDL_Color textColor = params.textColor;
    
    if (displayText.empty() && focusedField != &params) {
        displayText = params.placeholder;
        textColor = params.placeholderColor;
    }

    if (!params.multiLine) {
        std::string textBeforeCursor = params.value.substr(0, params.cursorPos);
        float2 sizeBefore = getTextSize(font, textBeforeCursor, params.textSize);
        float visibleWidth = params.rect.w - margin * 2.0f;
        
        if (sizeBefore.x - params.scrollOffset > visibleWidth) {
            params.scrollOffset = sizeBefore.x - visibleWidth;
        }
        if (sizeBefore.x - params.scrollOffset < 0) {
            params.scrollOffset = sizeBefore.x;
        }
        
        float2 textPos = { params.rect.x + margin - params.scrollOffset, params.rect.y + (params.rect.h - (float)params.textSize) / 2.0f };
        
        // Render selection
        if (params.hasSelection()) {
            auto [start, end] = params.getSelectionRange();
            std::string subLeft = params.value.substr(0, start);
            std::string subMid = params.value.substr(start, end - start);
            float2 sizeLeft = getTextSize(font, subLeft, params.textSize);
            float2 sizeMid = getTextSize(font, subMid, params.textSize);
            
            Rect selRect = { textPos.x + sizeLeft.x, textPos.y, sizeMid.x, (float)params.textSize };
            drawRect(selRect, params.selectionColor);
        }

        drawText(displayText, textPos, textColor, params.textSize);
        
        if (focusedField == &params && params.cursorVisible) {
            float2 cursorStart = { textPos.x + sizeBefore.x, textPos.y };
            float2 cursorEnd = { cursorStart.x, cursorStart.y + (float)params.textSize };
            drawLine(cursorStart, cursorEnd, params.textColor, 1);
        }
    } else {
        // Multi-line
        std::vector<std::string> lines;
        std::string line;
        std::stringstream ss(displayText);
        while (std::getline(ss, line, '\n')) {
            lines.push_back(line);
        }
        if (!displayText.empty() && displayText.back() == '\n') lines.push_back("");
        if (displayText.empty()) lines.push_back("");

        float lineSpacing = 4.0f;
        float currentY = params.rect.y + 4.0f - params.verticalScrollOffset;
        int charCount = 0;
        auto [selStart, selEnd] = params.getSelectionRange();
        
        for (size_t i = 0; i < lines.size(); ++i) {
            const auto& l = lines[i];
            float2 linePos = { params.rect.x + margin, currentY };
            int lineLen = (int)l.length();
            int lineEndIdxTotal = charCount + lineLen;
            
            // Render selection for this line
            if (params.hasSelection()) {
                int drawStart = std::max(selStart, charCount);
                int drawEnd = std::min(selEnd, lineEndIdxTotal);
                
                if (drawStart < drawEnd) {
                    std::string subLeft = l.substr(0, drawStart - charCount);
                    std::string subMid = l.substr(drawStart - charCount, drawEnd - drawStart);
                    float2 sizeLeft = getTextSize(font, subLeft, params.textSize);
                    float2 sizeMid = getTextSize(font, subMid, params.textSize);
                    
                    Rect selRect = { linePos.x + sizeLeft.x, linePos.y, sizeMid.x, (float)params.textSize };
                    drawRect(selRect, params.selectionColor);
                }
                
                // Selection of the newline itself
                if (selEnd > lineEndIdxTotal && i < lines.size() - 1 && selStart <= lineEndIdxTotal) {
                    float2 sizeWhole = getTextSize(font, l, params.textSize);
                    Rect selRect = { linePos.x + sizeWhole.x, linePos.y, 10.0f, (float)params.textSize };
                    drawRect(selRect, params.selectionColor);
                }
            }

            if (params.lineRenderer) {
                params.lineRenderer(*this, l, linePos, textColor, params.textSize);
            } else {
                drawText(l, linePos, textColor, params.textSize);
            }
            
            if (focusedField == &params && params.cursorVisible) {
                if (params.cursorPos >= charCount && params.cursorPos <= lineEndIdxTotal) {
                    // Avoid drawing cursor twice if it's at the newline of a wrapped line (though we don't have wrap yet)
                    // But here it's strictly \n based.
                    std::string sub = l.substr(0, params.cursorPos - charCount);
                    float2 subSize = getTextSize(font, sub, params.textSize);
                    float2 cursorStart = { linePos.x + subSize.x, currentY };
                    float2 cursorEnd = { cursorStart.x, cursorStart.y + (float)params.textSize };
                    drawLine(cursorStart, cursorEnd, params.textColor, 1);
                    
                    // Auto-scroll vertical
                    if (currentY < params.rect.y) params.verticalScrollOffset -= (params.rect.y - currentY);
                    if (currentY + params.textSize > params.rect.y + params.rect.h) params.verticalScrollOffset += (currentY + params.textSize - (params.rect.y + params.rect.h));
                }
            }
            charCount += lineLen + 1; // +1 for newline
            currentY += static_cast<float>(params.textSize) + lineSpacing;
        }
    }

    SDL_SetRenderClipRect(app.getRenderer(), nullptr);
    return;
}

void Renderer::drawInputField(InputField& params, LayoutManager& layout, float w, float h) {
    params.rect = layout.next(w, h);
    return drawInputField(params);
}

void Renderer::drawInputField(InputField& params, float w, float h) {
    params.rect = getNextRect(w, h);
    return drawInputField(params);
}

void Renderer::drawColorPicker(ColorPicker& params) {
    float2 mousePos = Input::getMousePos();
    bool isLMBDown = Input::isMouseButtonDown(SDL_BUTTON_LEFT);
    bool isLMBPressed = Input::isMouseButtonPressed(SDL_BUTTON_LEFT);

    // Layout
    float padding = 5.0f;
    float hueSliderWidth = 20.0f;
    Rect svRect = { params.rect.x, params.rect.y, params.rect.w - hueSliderWidth - padding, params.rect.h };
    Rect hueRect = { params.rect.x + params.rect.w - hueSliderWidth, params.rect.y, hueSliderWidth, params.rect.h };

    // Interaction
    if (params.enabled) {
        if (isLMBPressed) {
            if (Input::isMouseHoveringRect(mousePos, svRect)) params.isDraggingSV = true;
            if (Input::isMouseHoveringRect(mousePos, hueRect)) params.isDraggingHue = true;
        }

        if (!isLMBDown) {
            params.isDraggingSV = false;
            params.isDraggingHue = false;
        }

        if (params.isDraggingSV) {
            params.saturation = std::clamp((mousePos.x - svRect.x) / svRect.w, 0.0f, 1.0f);
            params.value = std::clamp(1.0f - (mousePos.y - svRect.y) / svRect.h, 0.0f, 1.0f);
            params.updateColorFromHSV();
        }

        if (params.isDraggingHue) {
            params.hue = std::clamp((mousePos.y - hueRect.y) / hueRect.h, 0.0f, 1.0f) * 360.0f;
            if (params.hue >= 360.0f) params.hue = 359.9f;
            params.updateColorFromHSV();
        }
    }

    // Drawing SV Square (simplified with triangles or a small texture if we had one, but let's use a procedural approach or just draw some lines)
    // Actually, drawing a full SV square procedurally every frame without a shader or texture is slow.
    // Let's draw it using 4 points and SDL_RenderGeometry?
    
    // Hue color for the top-right corner of SV square
    ColorPicker temp = params;
    temp.saturation = 1.0f; temp.value = 1.0f;
    temp.updateColorFromHSV();
    SDL_Color hueColor = temp.color;

    SDL_Vertex verts[4];
    // TL: White
    verts[0] = { { svRect.x, svRect.y }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0, 0 } };
    // TR: Hue Color
    verts[1] = { { svRect.x + svRect.w, svRect.y }, { hueColor.r / 255.0f, hueColor.g / 255.0f, hueColor.b / 255.0f, 1.0f }, { 0, 0 } };
    // BL: Black
    verts[2] = { { svRect.x, svRect.y + svRect.h }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0, 0 } };
    // BR: Black
    verts[3] = { { svRect.x + svRect.w, svRect.y + svRect.h }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0, 0 } };

    int indices[] = { 0, 1, 2, 1, 3, 2 };
    SDL_RenderGeometry(app.getRenderer(), nullptr, verts, 4, indices, 6);

    // Draw SV cursor
    float cursorX = svRect.x + params.saturation * svRect.w;
    float cursorY = svRect.y + (1.0f - params.value) * svRect.h;
    drawCircleOutline({ {cursorX, cursorY}, 5.0f }, (params.value > 0.5f ? Color::Black : Color::White), 2);

    // Draw Hue Slider
    for (int i = 0; i < (int)hueRect.h; ++i) {
        float h = (float)i / hueRect.h * 360.0f;
        ColorPicker hp; hp.hue = h; hp.saturation = 1.0f; hp.value = 1.0f;
        hp.updateColorFromHSV();
        drawLine({hueRect.x, hueRect.y + i}, {hueRect.x + hueRect.w, hueRect.y + i}, hp.color, 1);
    }
    
    // Draw Hue cursor
    float hueY = hueRect.y + (params.hue / 360.0f) * hueRect.h;
    drawRectOutline({ hueRect.x - 2, hueY - 2, hueRect.w + 4, 4 }, Color::White, 2);

    drawRectOutline(params.rect, Color::Border, 1);
}

void Renderer::drawColorPicker(ColorPicker& params, LayoutManager& layout, float w, float h) {
    params.rect = layout.next(w, h);
    drawColorPicker(params);
}

void Renderer::drawColorPicker(ColorPicker& params, float w, float h) {
    params.rect = getNextRect(w, h);
    drawColorPicker(params);
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
    Rect r = getNextRect(finalW, finalH);
    drawImage(image, r);
}

void Renderer::drawImage(const RawImage& image, Rect dst) {
    if (image.w <= 0 || image.h <= 0) return;

    uint64_t signature = (static_cast<uint64_t>(image.w) << 32) | image.h;
    signature ^= static_cast<uint64_t>(image.scale * 1000.0f);
    if (image.type == NoiseType::NONE) {
        signature ^= reinterpret_cast<uintptr_t>(image.data.data());
    }

    auto it = progressiveTextures.find(signature);
    if (it == progressiveTextures.end()) {
        ProgressiveTexture pt;
        SDL_TextureAccess access = (image.type == NoiseType::NONE) ? SDL_TEXTUREACCESS_STATIC : SDL_TEXTUREACCESS_STREAMING;

        SDL_Texture* sdlTex = SDL_CreateTexture(app.getRenderer(), SDL_PIXELFORMAT_RGBA32, access, image.w, image.h);
        if (!sdlTex) return;

        SDL_SetTextureScaleMode(sdlTex, SDL_SCALEMODE_PIXELART);

        pt.texture = Texture{ sdlTex, image.w, image.h };
        SDL_SetTextureBlendMode(pt.texture.handle, SDL_BLENDMODE_BLEND);

        if (image.type == NoiseType::NONE) {
            if (!image.data.empty()) {
                SDL_UpdateTexture(pt.texture.handle, nullptr, image.data.data(), image.w * sizeof(SDL_Color));
            }
            pt.currentY = image.h;
        } else {
            pt.currentY = 0;
            void* pixels = nullptr;
            int pitch = 0;
            if (SDL_LockTexture(pt.texture.handle, nullptr, &pixels, &pitch)) {
                std::memset(pixels, 0, static_cast<size_t>(pitch) * image.h);
                SDL_UnlockTexture(pt.texture.handle);
            }
        }

        progressiveTextures[signature] = pt;
        it = progressiveTextures.find(signature);
    }

    auto& pt = it->second;

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

    if (pt.texture.isValid()) {
        SDL_FRect dstFRect = { dst.x, dst.y, dst.w, dst.h };
        SDL_RenderTexture(app.getRenderer(), pt.texture.handle, nullptr, &dstFRect);
    }
}

void Renderer::drawImageRotated(const RawImage& image, Rect dst, float angle, float2 center) {
    if (image.w <= 0 || image.h <= 0) return;

    // Use same signature logic to find the texture
    uint64_t signature = (static_cast<uint64_t>(image.w) << 32) | image.h;
    signature ^= static_cast<uint64_t>(image.scale * 1000.0f);
    if (image.type == NoiseType::NONE) {
        signature ^= reinterpret_cast<uintptr_t>(image.data.data());
    }

    auto it = progressiveTextures.find(signature);
    if (it == progressiveTextures.end()) {
        // If not found, call drawImage once to initialize it
        drawImage(image, dst);
        it = progressiveTextures.find(signature);
        if (it == progressiveTextures.end()) return;
    }

    auto& pt = it->second;
    if (pt.texture.isValid()) {
        SDL_FRect dstFRect = { dst.x, dst.y, dst.w, dst.h };
        SDL_FPoint sdlCenter = { center.x, center.y };
        SDL_RenderTextureRotated(app.getRenderer(), pt.texture.handle, nullptr, &dstFRect, angle, (center.x < 0) ? nullptr : &sdlCenter, SDL_FLIP_NONE);
    }
}

void Renderer::drawImage(const RawImage& image, float2 pos, float scale) {
    drawImage(image, Rect{ pos.x, pos.y, static_cast<float>(image.w) * scale, static_cast<float>(image.h) * scale });
}

void Renderer::drawSprite(const Sprite& sprite, float scale) {
    if (!sprite.getTexture()) return;
    Rect r = { sprite.pos.x, sprite.pos.y, sprite.width * scale, sprite.height * scale };
    if (activeLayout) r = activeLayout->next(r.w, r.h);
    drawSprite(sprite, r);
}

void Renderer::drawSprite(const Sprite& sprite, float w, float h) {
    if (!sprite.getTexture()) return;
    Rect r = getNextRect(w, h);
    drawSprite(sprite, r);
}

void Renderer::drawSprite(const Sprite& sprite, Rect dst) {
    if (!sprite.getTexture()) return;
    SDL_FRect d = { dst.x, dst.y, dst.w, dst.h };
    SDL_RenderTexture(app.getRenderer(), sprite.getTexture(), nullptr, &d);
}

void Renderer::drawNineSlice(const NineSlice& ns, Rect dst) {
    if (!ns.isValid() || !ns.sprite->getTexture()) return;

    SDL_Texture* tex = ns.sprite->getTexture();
    float sw = ns.sprite->width;
    float sh = ns.sprite->height;

    // Source coordinates
    float s_left = (float)ns.left;
    float s_right = (float)ns.right;
    float s_top = (float)ns.top;
    float s_bottom = (float)ns.bottom;

    float s_mid_w = sw - s_left - s_right;
    float s_mid_h = sh - s_top - s_bottom;

    // Destination coordinates
    float d_left = s_left;
    float d_right = s_right;
    float d_top = s_top;
    float d_bottom = s_bottom;

    // Adjust destination margins if they exceed target size
    if (d_left + d_right > dst.w) {
        float scale = dst.w / (d_left + d_right);
        d_left *= scale;
        d_right *= scale;
    }
    if (d_top + d_bottom > dst.h) {
        float scale = dst.h / (d_top + d_bottom);
        d_top *= scale;
        d_bottom *= scale;
    }

    float d_mid_w = dst.w - d_left - d_right;
    float d_mid_h = dst.h - d_top - d_bottom;

    auto drawPart = [&](float sx, float sy, float sw_p, float sh_p, float dx, float dy, float dw_p, float dh_p) {
        if (sw_p <= 0 || sh_p <= 0 || dw_p <= 0 || dh_p <= 0) return;
        SDL_FRect srcRect = { sx, sy, sw_p, sh_p };
        SDL_FRect dstRect = { dst.x + dx, dst.y + dy, dw_p, dh_p };
        SDL_RenderTexture(app.getRenderer(), tex, &srcRect, &dstRect);
    };

    // Corners
    drawPart(0, 0, s_left, s_top, 0, 0, d_left, d_top); // TL
    drawPart(sw - s_right, 0, s_right, s_top, dst.w - d_right, 0, d_right, d_top); // TR
    drawPart(0, sh - s_bottom, s_left, s_bottom, 0, dst.h - d_bottom, d_left, d_bottom); // BL
    drawPart(sw - s_right, sh - s_bottom, s_right, s_bottom, dst.w - d_right, dst.h - d_bottom, d_right, d_bottom); // BR

    // Edges
    drawPart(s_left, 0, s_mid_w, s_top, d_left, 0, d_mid_w, d_top); // Top
    drawPart(s_left, sh - s_bottom, s_mid_w, s_bottom, d_left, dst.h - d_bottom, d_mid_w, d_bottom); // Bottom
    drawPart(0, s_top, s_left, s_mid_h, 0, d_top, d_left, d_mid_h); // Left
    drawPart(sw - s_right, s_top, s_right, s_mid_h, dst.w - d_right, d_top, d_right, d_mid_h); // Right

    // Center
    drawPart(s_left, s_top, s_mid_w, s_mid_h, d_left, d_top, d_mid_w, d_mid_h);
}

void Renderer::updateImage(const RawImage& image) {
    if (image.w <= 0 || image.h <= 0 || image.data.empty()) return;

    uint64_t signature = (static_cast<uint64_t>(image.w) << 32) | image.h;
    signature ^= static_cast<uint64_t>(image.scale * 1000.0f);
    if (image.type == NoiseType::NONE) {
        signature ^= reinterpret_cast<uintptr_t>(image.data.data());
    }

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

// void Renderer::applyBloom(float intensity) {
//     if (!sceneTexture || !bloomTexture1 || !bloomTexture2 || intensity <= 0.0f) return;
//
//     // Pass 1: Bright Pass / Extraction
//     // We want to isolate only the brightest parts of the scene.
//     // Since we don't have shaders, we can use SDL_BLENDMODE_MOD with a threshold color.
//     // Anything below the threshold will become much darker.
//     SDL_SetRenderTarget(app.getRenderer(), bloomTexture1);
//     SDL_SetRenderDrawColor(app.getRenderer(), 0, 0, 0, 255);
//     SDL_RenderClear(app.getRenderer());
//
//     // Draw the scene into bloomTexture1
//     SDL_SetTextureBlendMode(sceneTexture, SDL_BLENDMODE_NONE);
//     SDL_RenderTexture(app.getRenderer(), sceneTexture, nullptr, nullptr);
//
//     // Multiplicative blend with a "threshold" color (dark) to suppress low-intensity pixels
//     // Using a dark gray means only very bright pixels survive with significant intensity
//     SDL_SetRenderDrawColor(app.getRenderer(), 40, 40, 40, 255); // Threshold factor
//     SDL_SetRenderDrawBlendMode(app.getRenderer(), SDL_BLENDMODE_MOD);
//     SDL_FRect fullRect = { 0, 0, (float)bloomW, (float)bloomH };
//     SDL_RenderFillRect(app.getRenderer(), &fullRect);
//     SDL_SetRenderDrawBlendMode(app.getRenderer(), SDL_BLENDMODE_NONE); // Reset blend mode
//     SDL_SetTextureBlendMode(sceneTexture, SDL_BLENDMODE_BLEND); // Ensure sceneTexture isn't stuck in NONE
//
//     // Pass 2: Blur iterations
//     // We'll use bloomTexture2 as temporary buffer
//     // More iterations and varying offsets for a smoother, larger bloom
//     blurTexture(bloomTexture1, bloomTexture2, 5);
//
//     // Final Pass: Additive blend back to scene
//     SDL_SetRenderTarget(app.getRenderer(), sceneTexture);
//     SDL_SetTextureBlendMode(bloomTexture1, SDL_BLENDMODE_ADD);
//
//     // We can layer it multiple times for extra "oomph"
//     float alpha = std::clamp(intensity, 0.0f, 1.0f);
//     SDL_SetTextureAlphaMod(bloomTexture1, (Uint8)(alpha * 255));
//     SDL_RenderTexture(app.getRenderer(), bloomTexture1, nullptr, nullptr);
//
//     // Layer 2: slightly larger and softer
//     SDL_SetTextureAlphaMod(bloomTexture1, (Uint8)(alpha * 180));
//     SDL_RenderTexture(app.getRenderer(), bloomTexture1, nullptr, nullptr);
//
//     // Layer 3: smaller, more intense core
//     SDL_SetTextureAlphaMod(bloomTexture1, (Uint8)(alpha * 128));
//     SDL_RenderTexture(app.getRenderer(), bloomTexture1, nullptr, nullptr);
//
//     // Restore state
//     SDL_SetTextureBlendMode(bloomTexture1, SDL_BLENDMODE_NONE); // Reset texture blend mode
//     SDL_SetRenderTarget(app.getRenderer(), sceneTexture);
// }

void Renderer::ensureBloomTextures() {
    int w = app.getWidth();
    int h = app.getHeight();

    if (!sceneTexture || w != bloomW * 4 || h != bloomH * 4) {
        if (sceneTexture) SDL_DestroyTexture(sceneTexture);
        if (bloomTexture1) SDL_DestroyTexture(bloomTexture1);
        if (bloomTexture2) SDL_DestroyTexture(bloomTexture2);

        sceneTexture = SDL_CreateTexture(app.getRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
        
        bloomW = w / 4;
        bloomH = h / 4;
        if (bloomW < 1) bloomW = 1;
        if (bloomH < 1) bloomH = 1;

        bloomTexture1 = SDL_CreateTexture(app.getRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bloomW, bloomH);
        bloomTexture2 = SDL_CreateTexture(app.getRenderer(), SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, bloomW, bloomH);
    }
}

void Renderer::blurTexture(SDL_Texture* target, SDL_Texture* temp, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        // Exponentially increasing offset for a wider, softer blur
        float offset = std::pow(1.5f, (float)i);
        
        // Horizontal blur
        SDL_SetRenderTarget(app.getRenderer(), temp);
        SDL_SetRenderDrawColor(app.getRenderer(), 0, 0, 0, 255);
        SDL_RenderClear(app.getRenderer());
        SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(target, 160); // Slightly more opaque for faster accumulation
        
        SDL_FRect rLeft = {-offset, 0, (float)bloomW, (float)bloomH};
        SDL_FRect rRight = {offset, 0, (float)bloomW, (float)bloomH};
        SDL_RenderTexture(app.getRenderer(), target, nullptr, &rLeft);
        SDL_RenderTexture(app.getRenderer(), target, nullptr, &rRight);

        // Vertical blur
        SDL_SetRenderTarget(app.getRenderer(), target);
        SDL_SetRenderDrawColor(app.getRenderer(), 0, 0, 0, 255);
        SDL_RenderClear(app.getRenderer());
        SDL_SetTextureBlendMode(temp, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(temp, 160);
        
        SDL_FRect rUp = {0, -offset, (float)bloomW, (float)bloomH};
        SDL_FRect rDown = {0, offset, (float)bloomW, (float)bloomH};
        SDL_RenderTexture(app.getRenderer(), temp, nullptr, &rUp);
        SDL_RenderTexture(app.getRenderer(), temp, nullptr, &rDown);
    }
    // Final cleanup of texture state
    SDL_SetTextureBlendMode(target, SDL_BLENDMODE_NONE);
    SDL_SetTextureAlphaMod(target, 255);
    SDL_SetTextureBlendMode(temp, SDL_BLENDMODE_NONE);
    SDL_SetTextureAlphaMod(temp, 255);
}

void Renderer::screenShake(float durationInSeconds, float intensity, float frequency) {
    this->shakeTimer = durationInSeconds;
    this->shakeIntensity = intensity;
    this->shakeFrequency = frequency;
    this->shakeTick = 0.0f;
}

void Renderer::applyScreenShake(float dt) {
    if (shakeTimer <= 0.0f) {
        return;
    }

    shakeTimer -= dt;
    shakeTick += dt * shakeFrequency;

    float offsetX = std::sin(shakeTick) * shakeIntensity * (std::rand() % 2 == 0 ? 1.0f : -1.0f);
    float offsetY = std::cos(shakeTick) * shakeIntensity * (std::rand() % 2 == 0 ? 1.0f : -1.0f);

    SDL_Rect baseViewport;
    if (SDL_GetRenderViewport(app.getRenderer(), &baseViewport)) {
        baseViewport.x += static_cast<int>(offsetX);
        baseViewport.y += static_cast<int>(offsetY);
        SDL_SetRenderViewport(app.getRenderer(), &baseViewport);
    }
}

void Renderer::resetScreenShake() {
    SDL_Rect defaultViewport = { 0, 0, app.getWidth(), app.getHeight() };
    SDL_SetRenderViewport(app.getRenderer(), &defaultViewport);
}


void Renderer::drawPanel(UI_Panel& panel) {
    if (app.isAutoResizeEnabled()) {
        if (panel.area.w == (float)app.getInitialWidth()) panel.area.w = (float)app.getWidth();
        if (panel.area.h == (float)app.getInitialHeight()) panel.area.h = (float)app.getHeight();
        if (panel.area.x == (float)app.getInitialWidth()) panel.area.x = (float)app.getWidth();
        if (panel.area.y == (float)app.getInitialHeight()) panel.area.y = (float)app.getHeight();
    }
    panel.resetLayout();
    drawRect(panel.area, panel.bgColor);
    if (!panel.outlineProperties.enabled) return;

    drawRectOutline(panel.area, panel.outlineProperties.color, panel.outlineProperties.width);
}
