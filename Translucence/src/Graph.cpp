#include "Graph.hpp"
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>

static std::string formatTickValue(float val) {
    if (std::abs(val - std::round(val)) < 0.0005f) {
        return std::to_string(static_cast<int>(std::round(val)));
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.3f", val);
    std::string s(buf);
    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') {
            s.pop_back();
        }
        if (!s.empty() && s.back() == '.') {
            s.pop_back();
        }
    }
    return s;
}

static std::vector<float> getNiceTicks(float minVal, float maxVal, int targetCount, bool forceInteger) {
    std::vector<float> ticks;
    float range = maxVal - minVal;
    if (range <= 1e-6f) {
        ticks.push_back(minVal);
        return ticks;
    }
    float rawStep = range / static_cast<float>(targetCount);
    float power = std::floor(std::log10(rawStep));
    float normStep = rawStep / std::pow(10.0f, power);
    
    float niceNormStep;
    if (normStep < 1.5f) niceNormStep = 1.0f;
    else if (normStep < 2.25f) niceNormStep = 2.0f;
    else if (normStep < 3.5f) niceNormStep = 2.5f;
    else if (normStep < 7.5f) niceNormStep = 5.0f;
    else niceNormStep = 10.0f;
    
    float niceStep = niceNormStep * std::pow(10.0f, power);
    if (forceInteger) {
        if (niceStep < 1.0f) {
            niceStep = 1.0f;
        } else {
            niceStep = std::ceil(niceStep);
        }
    }
    
    // Generate ticks starting from first nice value >= minVal
    float startTick = std::ceil(minVal / niceStep) * niceStep;
    float eps = niceStep * 1e-4f;
    for (float val = startTick; val <= maxVal + eps; val += niceStep) {
        if (val > maxVal + eps) break;
        ticks.push_back(val);
    }
    return ticks;
}

float2 GraphPlot::dataToScreen(float x, float y) const {
    float px = (x - xMin) / (xMax - xMin);
    float py = (y - yMin) / (yMax - yMin);
    // y axis grows downward on screen
    float sx = area.x + px * area.w;
    float sy = area.y + (1.0f - py) * area.h;
    return {sx, sy};
}

float2 GraphPlot::getTextDimensions(const std::string& text, int size) const {
    TTF_Font* font = r.getApp().getFont();
    if (!font) {
        return {static_cast<float>(text.size()) * (size * 0.6f), static_cast<float>(size)};
    }
    return getTextSize(font, text, size);
}

void GraphPlot::fitToWindow(const Application& app, float marginPx) {
    float w = static_cast<float>(app.getWidth());
    float h = static_cast<float>(app.getHeight());

    // Dynamically calculate asymmetric margins based on marginPx
    float left = marginPx * 1.2f;
    float right = marginPx * 0.8f;
    float top = marginPx * 1.4f;
    float bottom = marginPx * 1.5f;

    // Measure label sizes to make margin calculation perfectly dynamic!
    float yTitleW = 0.0f;
    if (!yLabel.empty()) {
        yTitleW = getTextDimensions(yLabel, std::clamp(static_cast<int>(marginPx * 0.25f), 11, 28)).x;
    }

    float maxYLabelW = 30.0f;
    if (!ys.empty()) {
        auto [ymin, ymax] = std::minmax_element(ys.begin(), ys.end());
        float maxVal = std::max(std::abs(*ymin), std::abs(*ymax));
        std::string testStr = formatTickValue(maxVal);
        maxYLabelW = getTextDimensions(testStr, std::clamp(static_cast<int>(marginPx * 0.22f), 10, 24)).x;
    }

    // Ensure margins are sufficient for rendering labels, title and hint text
    left = std::max({left, yTitleW * 0.5f + 15.0f, maxYLabelW + marginPx * 0.4f + 15.0f, 85.0f});
    right = std::max(right, 45.0f);
    top = std::max(top, 100.0f);
    bottom = std::max(bottom, 110.0f);

    area = {left, top, std::max(0.0f, w - left - right), std::max(0.0f, h - top - bottom)};
}

void GraphPlot::fitToWindow(const Application& app) {
    if (hasCustomMargins) {
        float w = static_cast<float>(app.getWidth());
        float h = static_cast<float>(app.getHeight());
        area = {marginLeft, marginTop, std::max(0.0f, w - marginLeft - marginRight), std::max(0.0f, h - marginTop - marginBottom)};
    } else {
        // Fallback to a nice, robust default base margin
        fitToWindow(app, 70.0f);
    }
}

void GraphPlot::draw() {
    if (autoFit) {
        fitToWindow(r.getApp());
    }

    // Draw background panel
    r.drawRoundedRect(area, Color::BgElevated, 8);
    r.drawRoundedRectOutline(area, Color::Border, 8, 1);

    // Dynamic text sizes
    int tickTextSize = std::clamp(static_cast<int>(((area.w + area.h) * 0.5f) * 0.022f * textScale), 10, 24);
    int labelTextSize = std::clamp(static_cast<int>(tickTextSize * 1.05f), 11, 28);
    float tickLength = std::max(6.0f, static_cast<float>(tickTextSize) * 0.7f);

    // Determine if we should force integer ticks
    bool xIsInt = forceXInt;
    if (!xIsInt && !xs.empty()) {
        xIsInt = true;
        for (float val : xs) {
            if (std::abs(val - std::round(val)) > 1e-4f) {
                xIsInt = false;
                break;
            }
        }
    }
    
    bool yIsInt = forceYInt;
    if (!yIsInt && !ys.empty()) {
        yIsInt = true;
        for (float val : ys) {
            if (std::abs(val - std::round(val)) > 1e-4f) {
                yIsInt = false;
                break;
            }
        }
    }

    // Generate ticks
    std::vector<float> xTicks;
    if (manualXSeg > 0) {
        for (int i = 0; i <= manualXSeg; ++i) {
            xTicks.push_back(xMin + static_cast<float>(i) * (xMax - xMin) / static_cast<float>(manualXSeg));
        }
    } else {
        int targetXCount = std::clamp(static_cast<int>(area.w / pixelsPerTick), 3, 12);
        xTicks = getNiceTicks(xMin, xMax, targetXCount, xIsInt);
    }

    std::vector<float> yTicks;
    if (manualYSeg > 0) {
        for (int i = 0; i <= manualYSeg; ++i) {
            yTicks.push_back(yMin + static_cast<float>(i) * (yMax - yMin) / static_cast<float>(manualYSeg));
        }
    } else {
        int targetYCount = std::clamp(static_cast<int>(area.h / pixelsPerTick), 3, 12);
        yTicks = getNiceTicks(yMin, yMax, targetYCount, yIsInt);
    }

    // Safety fallback
    if (xTicks.size() < 2) {
        xTicks.clear();
        xTicks.push_back(xMin);
        xTicks.push_back(xMax);
    }
    if (yTicks.size() < 2) {
        yTicks.clear();
        yTicks.push_back(yMin);
        yTicks.push_back(yMax);
    }

    // Soft semi-transparent grid lines color
    SDL_Color gridColor = Color::withAlpha(tickColor, 60);

    // 1. Draw grid lines (optional, light) - draw them first so they appear under axes and plots
    // Vertical grid lines
    for (float val : xTicks) {
        float t = (val - xMin) / (xMax - xMin);
        if (t > 0.001f && t < 0.999f) {
            float sx = area.x + t * area.w;
            r.drawLine({sx, area.y}, {sx, area.y + area.h}, gridColor, 1);
        }
    }
    // Horizontal grid lines
    for (float val : yTicks) {
        float t = (val - yMin) / (yMax - yMin);
        if (t > 0.001f && t < 0.999f) {
            float sy = area.y + (1.0f - t) * area.h;
            r.drawLine({area.x, sy}, {area.x + area.w, sy}, gridColor, 1);
        }
    }

    // 2. Draw axis lines (bottom X, left Y)
    float2 x0 = {area.x, area.y + area.h};
    float2 x1 = {area.x + area.w, area.y + area.h};
    float2 y0 = {area.x, area.y};
    float2 y1 = {area.x, area.y + area.h};
    r.drawLine(x0, x1, axisColor, 2);
    r.drawLine(y1, y0, axisColor, 2);

    // X ticks & labels overlapping check
    int xLabelSkip = 1;
    if (xTicks.size() >= 2) {
        float minTickDist = area.w;
        for (size_t i = 1; i < xTicks.size(); ++i) {
            float dist = ((xTicks[i] - xTicks[i-1]) / (xMax - xMin)) * area.w;
            if (dist < minTickDist) minTickDist = dist;
        }
        float maxLabelW = 0.0f;
        for (float val : xTicks) {
            float labelW = getTextDimensions(formatTickValue(val), tickTextSize).x;
            if (labelW > maxLabelW) maxLabelW = labelW;
        }
        while (xLabelSkip < 10 && (maxLabelW + 16.0f) > (minTickDist * xLabelSkip)) {
            xLabelSkip++;
        }
    }

    // Y ticks & labels overlapping check
    int yLabelSkip = 1;
    if (yTicks.size() >= 2) {
        float minTickDist = area.h;
        for (size_t i = 1; i < yTicks.size(); ++i) {
            float dist = ((yTicks[i] - yTicks[i-1]) / (yMax - yMin)) * area.h;
            if (dist < minTickDist) minTickDist = dist;
        }
        float labelH = static_cast<float>(tickTextSize);
        while (yLabelSkip < 10 && (labelH + 8.0f) > (minTickDist * yLabelSkip)) {
            yLabelSkip++;
        }
    }

    // 3. Draw ticks and tick labels
    // X ticks & labels
    for (size_t i = 0; i < xTicks.size(); ++i) {
        float val = xTicks[i];
        float t = (val - xMin) / (xMax - xMin);
        if (t >= -0.001f && t <= 1.001f) {
            float sx = area.x + t * area.w;
            // Draw tick mark line downwards
            r.drawLine({sx, area.y + area.h}, {sx, area.y + area.h + tickLength}, axisColor, 1);
            
            if (i % xLabelSkip == 0) {
                // Format and draw tick label centered under tick
                std::string labelStr = formatTickValue(val);
                float labelW = getTextDimensions(labelStr, tickTextSize).x;
                float2 labelPos = {sx - labelW * 0.5f, area.y + area.h + tickLength + 4.0f};
                r.drawText(labelStr, labelPos, textColor, tickTextSize);
            }
        }
    }

    // Y ticks & labels
    for (size_t i = 0; i < yTicks.size(); ++i) {
        float val = yTicks[i];
        float t = (val - yMin) / (yMax - yMin);
        if (t >= -0.001f && t <= 1.001f) {
            float sy = area.y + (1.0f - t) * area.h;
            // Draw tick mark line leftwards
            r.drawLine({area.x, sy}, {area.x - tickLength, sy}, axisColor, 1);
            
            if (i % yLabelSkip == 0) {
                // Format and draw tick label centered vertically to the left
                std::string labelStr = formatTickValue(val);
                float labelW = getTextDimensions(labelStr, tickTextSize).x;
                float2 labelPos = {area.x - tickLength - labelW - 6.0f, sy - static_cast<float>(tickTextSize) * 0.5f};
                r.drawText(labelStr, labelPos, textColor, tickTextSize);
            }
        }
    }

    // 4. Draw data plot
    if (xs.size() == ys.size() && !xs.empty()) {
        // Draw lines first (under points)
        if (drawLines && xs.size() >= 2) {
            for (size_t i = 1; i < xs.size(); ++i) {
                float2 a = dataToScreen(xs[i - 1], ys[i - 1]);
                float2 b = dataToScreen(xs[i], ys[i]);
                r.drawLine(a, b, lineColor, lineThickness);
            }
        }

        // Draw points with a clean halo/border over lines
        for (size_t i = 0; i < xs.size(); ++i) {
            float2 p = dataToScreen(xs[i], ys[i]);
            r.drawCircle({p, pointRadius + 1.5f}, Color::BgElevated);
            r.drawCircle({p, pointRadius}, pointColor);
        }
    }

    // 5. Draw axis titles (avoiding all overlaps)
    if (!xLabel.empty()) {
        float w = getTextDimensions(xLabel, labelTextSize).x;
        // Bottom centering, below tick labels
        float2 pos = {area.x + area.w * 0.5f - w * 0.5f, area.y + area.h + tickLength + 4.0f + static_cast<float>(tickTextSize) + 10.0f};
        r.drawText(xLabel, pos, Color::TextPrimary, labelTextSize);
    }
    if (!yLabel.empty()) {
        float w = getTextDimensions(yLabel, labelTextSize).x;
        // Centered horizontally above the left axis (Y axis) line
        float2 pos = {area.x - w * 0.5f, area.y - static_cast<float>(labelTextSize) - 10.0f};
        r.drawText(yLabel, pos, Color::TextPrimary, labelTextSize);
    }

    // 6. Draw Title & Hint if present
    if (!title.empty()) {
        int titleSize = std::clamp(static_cast<int>(labelTextSize * 1.15f), 16, 36);
        // Position it at the top left, roughly aligned with 30px from left
        r.drawText(title, {30.0f, std::max(10.0f, area.y * 0.35f)}, Color::TextPrimary, titleSize);
    }
    if (!hint.empty()) {
        int hintSize = std::clamp(static_cast<int>(tickTextSize * 0.9f), 10, 18);
        // Position it at the bottom left
        float windowH = static_cast<float>(r.getApp().getHeight());
        r.drawText(hint, {30.0f, windowH - std::max(20.0f, (windowH - (area.y + area.h)) * 0.45f)}, Color::TextMuted, hintSize);
    }
}
