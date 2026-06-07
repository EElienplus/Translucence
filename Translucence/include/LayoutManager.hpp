//
// Created by Stěpán Toman on 12.05.2026.
//

#ifndef TRANSLUCENCEWORKSPACE_LAYOUTMANAGER_HPP
#define TRANSLUCENCEWORKSPACE_LAYOUTMANAGER_HPP

#include "T_Core.hpp"
#include <vector>

/**
 * @brief Automatically handles the position and sizes of UI elements.
 */
class LayoutManager {
public:
    enum class Direction {
        Vertical,
        Horizontal
    };

    enum class Align {
        Start,
        Center,
        End
    };

    struct State {
        Rect container;
        Direction direction;
        float padding;
        float spacing;
        float margin;
        float2 cursor;
        Align hAlign = Align::Start;
        Align vAlign = Align::Start;
    };

    LayoutManager() = default;

    /**
     * @brief Initialize layout with application dimensions.
     */
    explicit LayoutManager(class Application& app) : app(&app) {
        begin({ 0, 0, (float)app.getWidth(), (float)app.getHeight() });
        app.registerLayoutManager(this);
    }

    ~LayoutManager() {
        if (app) {
            app->unregisterLayoutManager(this);
        }
    }

    /**
     * @brief Begins a new layout group.
     * @param container The area to layout within.
     * @param dir The direction of the layout.
     * @param padding Padding from the container edges.
     * @param spacing Spacing between elements.
     * @param margin External margin.
     */
    void begin(Rect container, Direction dir = Direction::Vertical, float padding = 10.0f, float spacing = 5.0f, float margin = 0.0f) {
        stack.clear();
        current.container = container;
        current.direction = dir;
        current.padding = padding;
        current.spacing = spacing;
        current.margin = margin;
        current.cursor = { container.x + padding + margin, container.y + padding + margin };
        current.hAlign = Align::Start;
        current.vAlign = Align::Start;
    }

    /**
     * @brief Pushes a new nested layout group.
     */
    void push(Direction dir, float w = -1, float h = -1, float padding = 0, float spacing = -1) {
        stack.push_back(current);
        
        Rect subArea = next(w, h);
        
        current.container = subArea;
        current.direction = dir;
        current.padding = padding;
        current.spacing = (spacing < 0) ? current.spacing : spacing;
        current.cursor = { subArea.x + padding, subArea.y + padding };
    }

    /**
     * @brief Convenience for pushing a horizontal group.
     */
    void row(float h = -1, float padding = 0, float spacing = -1) {
        push(Direction::Horizontal, -1, h, padding, spacing);
    }

    /**
     * @brief Convenience for pushing a vertical group.
     */
    void column(float w = -1, float padding = 0, float spacing = -1) {
        push(Direction::Vertical, w, -1, padding, spacing);
    }

    /**
     * @brief Ends the current nested layout group.
     */
    void pop() {
        if (!stack.empty()) {
            current = stack.back();
            stack.pop_back();
        }
    }

    /**
     * @brief Alias for pop().
     */
    void end() { pop(); }

    /**
     * @brief Gets the next available area in the layout.
     * @param w Requested width (-1 for Fill, 0-1 for relative if requested).
     * @param h Requested height (-1 for Fill, 0-1 for relative if requested).
     * @return The calculated Rect.
     */
    Rect next(float w = -1, float h = -1) {
        float availW, availH;
        
        // Calculate available space based on direction and cursor
        if (current.direction == Direction::Vertical) {
            availW = current.container.w - (current.padding * 2);
            availH = (current.container.y + current.container.h - current.padding) - current.cursor.y;
        } else {
            availW = (current.container.x + current.container.w - current.padding) - current.cursor.x;
            availH = current.container.h - (current.padding * 2);
        }

        float finalW = (w < 0) ? availW : w;
        float finalH = (h < 0) ? availH : h;

        // Check for relative sizing (if w/h is between 0 and 1, we could treat it as percentage? 
        // No, let's stick to explicit nextRelative for now to avoid ambiguity, or use a threshold)
        
        Rect r = { current.cursor.x, current.cursor.y, finalW, finalH };

        // Apply alignment within the "line" (orthogonal to direction)
        if (current.direction == Direction::Vertical) {
            float lineW = current.container.w - (current.padding * 2);
            if (current.hAlign == Align::Center) r.x = current.container.x + current.padding + (lineW - finalW) * 0.5f;
            else if (current.hAlign == Align::End) r.x = current.container.x + current.container.w - current.padding - finalW;
            
            current.cursor.y += finalH + current.spacing;
        } else {
            float lineH = current.container.h - (current.padding * 2);
            if (current.vAlign == Align::Center) r.y = current.container.y + current.padding + (lineH - finalH) * 0.5f;
            else if (current.vAlign == Align::End) r.y = current.container.y + current.container.h - current.padding - finalH;

            current.cursor.x += finalW + current.spacing;
        }

        return r;
    }

    /**
     * @brief Gets the next area as a fraction of remaining space.
     * @param weight Fraction of available space (0.0 - 1.0).
     */
    Rect nextWeighted(float weight) {
        if (current.direction == Direction::Vertical) {
            float availH = (current.container.y + current.container.h - current.padding) - current.cursor.y;
            return next(-1, availH * weight);
        } else {
            float availW = (current.container.x + current.container.w - current.padding) - current.cursor.x;
            return next(availW * weight, -1);
        }
    }

    /**
     * @brief Gets the next area using relative sizing (0.0 - 1.0) of the container.
     */
    Rect nextRelative(float rw, float rh) {
        float totalW = current.container.w - (current.padding * 2);
        float totalH = current.container.h - (current.padding * 2);
        return next(rw >= 0 ? totalW * rw : -1, rh >= 0 ? totalH * rh : -1);
    }

    void setAlign(Align h, Align v) {
        current.hAlign = h;
        current.vAlign = v;
    }

    void space(float amount) {
        if (current.direction == Direction::Vertical) current.cursor.y += amount;
        else current.cursor.x += amount;
    }

    void reset(Rect container, Direction dir = Direction::Vertical) {
        begin(container, dir, current.padding, current.spacing, current.margin);
    }

    const State& getState() const { return current; }
    const std::vector<State>& getStateStack() const { return stack; }
    Rect getContainer() const { return current.container; }

private:
    class Application* app = nullptr;
    State current;
    std::vector<State> stack;
};

#endif //TRANSLUCENCEWORKSPACE_LAYOUTMANAGER_HPP
