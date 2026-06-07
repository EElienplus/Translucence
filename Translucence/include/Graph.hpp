#ifndef TRANSLUCENCEWORKSPACE_GRAPH_HPP
#define TRANSLUCENCEWORKSPACE_GRAPH_HPP

#include "Renderer.hpp"
#include "Application.hpp"
#include "Input.hpp"

// A tiny, simple plotting helper that draws axes, points, and optional connecting lines.
// Keep API minimal to match project style.
class GraphPlot {
public:
    explicit GraphPlot(Renderer& r) : r(r) {}

    // Area on screen where the plot will be drawn
    void setArea(Rect rect) { area = rect; }

    // Fit plot area to current window with a margin (call every frame on resize)
    void fitToWindow(const Application& app, float marginPx);

    // Fit plot area to current window automatically using either custom or dynamic smart margins
    void fitToWindow(const Application& app);

    // Set custom asymmetric margins around the plot area
    void setMargins(float left, float right, float top, float bottom) {
        marginLeft = left; marginRight = right; marginTop = top; marginBottom = bottom;
        hasCustomMargins = true;
    }

    // Configure numeric axes and labels
    void setAxes(float xMin, float xMax, float yMin, float yMax,
                 const std::string& xLabel = "", const std::string& yLabel = "") {
        this->xMin = xMin; this->xMax = xMax; this->yMin = yMin; this->yMax = yMax;
        this->xLabel = xLabel; this->yLabel = yLabel;
    }

    // Only change labels
    void setLabels(const std::string& xLbl, const std::string& yLbl) {
        xLabel = xLbl;
        yLabel = yLbl;
    }

    // Set a title and hint to be drawn automatically
    void setTitle(const std::string& t) { title = t; }
    void setHint(const std::string& h) { hint = h; }

    // Enable/disable automatic fitting to window every frame during draw()
    void setAutoFit(bool enabled) { autoFit = enabled; }

    // Compute axes from current data with a little padding
    void autoAxesFromData(float paddingFrac = 0.05f) {
        if (xs.empty() || ys.empty()) return;
        auto [xmin, xmax] = std::minmax_element(xs.begin(), xs.end());
        auto [ymin, ymax] = std::minmax_element(ys.begin(), ys.end());
        float dx = std::max(1e-6f, *xmax - *xmin);
        float dy = std::max(1e-6f, *ymax - *ymin);
        float px = dx * paddingFrac;
        float py = dy * paddingFrac;
        xMin = *xmin - px; xMax = *xmax + px;
        yMin = *ymin - py; yMax = *ymax + py;

        // Auto clean integer bounds if appropriate
        bool xIsInt = forceXInt;
        if (!xIsInt) {
            xIsInt = true;
            for (float val : xs) {
                if (std::abs(val - std::round(val)) > 1e-4f) { xIsInt = false; break; }
            }
        }
        if (xIsInt) {
            xMin = std::floor(xMin);
            xMax = std::ceil(xMax);
        }

        bool yIsInt = forceYInt;
        if (!yIsInt) {
            yIsInt = true;
            for (float val : ys) {
                if (std::abs(val - std::round(val)) > 1e-4f) { yIsInt = false; break; }
            }
        }
        if (yIsInt) {
            yMin = std::floor(yMin);
            if (*ymin >= 0.0f && yMin < 0.0f) {
                yMin = 0.0f;
            }
            yMax = std::ceil(yMax);
        }
    }

    // Data as Y series with implicit X = [0..N-1]
    void setData(const std::vector<float>& ys) {
        xs.resize(ys.size());
        for (size_t i = 0; i < ys.size(); ++i) xs[i] = static_cast<float>(i);
        this->ys = ys;
    }

    // Data as explicit X,Y series (sizes must match)
    void setData(const std::vector<float>& xs, const std::vector<float>& ys) {
        this->xs = xs; this->ys = ys;
    }

    void setDrawLines(bool enabled) { drawLines = enabled; }
    [[nodiscard]] bool isDrawLines() const { return drawLines; }
    void toggleLines() { drawLines = !drawLines; }

    // Colors and styling
    void setColors(SDL_Color pointCol, SDL_Color lineCol) { pointColor = pointCol; lineColor = lineCol; }
    void setPointRadius(float r) { pointRadius = r; }
    void setLineThickness(int t) { lineThickness = t; }

    // Tick density and text scaling
    void setPixelsPerTick(float pixels) { pixelsPerTick = std::max(40.0f, pixels); }
    void setTextScale(float scale) { textScale = std::clamp(scale, 0.5f, 2.0f); }
    void setTickSegments(int xSeg, int ySeg) { manualXSeg = std::max(0, xSeg); manualYSeg = std::max(0, ySeg); }
    void setIntegerTicks(bool xInt, bool yInt) { forceXInt = xInt; forceYInt = yInt; }

    // Renders the plot (call every frame inside the app loop)
    void draw();

private:
    Renderer& r;
    Rect area{80, 60, 640, 420};

    float xMin = 0, xMax = 1;
    float yMin = 0, yMax = 1;
    std::string xLabel;
    std::string yLabel;
    std::string title;
    std::string hint;

    std::vector<float> xs;
    std::vector<float> ys;

    bool drawLines = true;
    bool autoFit = true;
    SDL_Color axisColor = Color::Border;
    SDL_Color tickColor = Color::Border;
    SDL_Color textColor = Color::TextMuted;
    SDL_Color pointColor = Color::Accent;
    SDL_Color lineColor = Color::Accent;
    float pointRadius = 3.0f;
    int lineThickness = 2;

    // Auto layout/ticks
    float pixelsPerTick = 100.0f; // desired pixels per major tick (auto)
    float textScale = 1.0f;       // relative text/UI scale for this plot
    int manualXSeg = 0;           // if >0, override auto segments
    int manualYSeg = 0;
    bool forceXInt = false;
    bool forceYInt = false;

    // Asymmetric margins and customization
    float marginLeft = 80.0f;
    float marginRight = 50.0f;
    float marginTop = 100.0f;
    float marginBottom = 110.0f;
    bool hasCustomMargins = false;

    // Helpers
    [[nodiscard]] float2 dataToScreen(float x, float y) const;
    [[nodiscard]] float2 getTextDimensions(const std::string& text, int size) const;
};

#endif //TRANSLUCENCEWORKSPACE_GRAPH_HPP
