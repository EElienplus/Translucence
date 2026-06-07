# Translucence Documentation

A lightweight C++ framework for graphics, audio, and UI built on SDL3.

## Getting Started

```cpp
#include <Translucence.hpp>

int main() {
    Application app;
    app.create(1280, 720, "Application Title");
    
    Renderer render(app);
    EventSystem events(app);

    while (app.isRunning()) {
        events.runEvents();
        
        render.clearBackground(Color::BgDeep);
        
        render.drawText("Hello Translucence", {50, 50}, Color::TextPrimary, 30);
        
        render.render();
    }
    return 0;
}
```

---

## Core Components

### Application (Application.hpp)
Manages the window context, main loop, and timing.
- `app.create(width, height, title)`: Initializes the window.
- `app.isRunning()`: Returns true until the application is closed.
- `app.getDeltaTime()`: Returns time since last frame in seconds.
- `app.getWidth()`, `app.getHeight()`: Returns current window dimensions.
- `app.setResizable(bool)`: Enables or disables window resizing.
- `app.getFPS()`: Returns current frames per second.

### Renderer (Renderer.hpp)
The primary interface for drawing operations.
- `render.clearBackground(color)`: Fills the screen with a color.
- `render.render()`: Finalizes and presents the current frame.
- `render.screenShake(duration, intensity, frequency)`: Applies a screen shake effect.
- `render.drawPanel(UI_Panel)`: Draws a UI panel container.

### EventSystem (EventSystem.hpp)
Processes window and input events.
- `events.runEvents()`: Polls and handles SDL events.
- `events.addModule(callback)`: Registers a custom event handler.

---

## Graphics and Drawing

### Basic Shapes
- `render.drawRect(Rect, Color)`
- `render.drawRectOutline(Rect, Color, thickness)`
- `render.drawRoundedRect(Rect, Color, radius)`
- `render.drawRoundedRectOutline(Rect, Color, radius, thickness)`
- `render.drawCircle(Circle, Color)`
- `render.drawCircleOutline(Circle, Color, thickness)`
- `render.drawTriangle(Triangle, Color)`
- `render.drawTriangleOutline(Triangle, Color, thickness)`
- `render.drawLine(startPos, endPos, Color, thickness)`
- `render.drawBezier(start, end, controlPoints, Color, thickness)`
- `render.drawShape(Shape, Color)`

### Sprites and Textures (Sprite.hpp)
- `Sprite sprite(app, "path.png")`: Loads an image.
- `render.drawSprite(sprite, scale)`: Draws a sprite with scaling.
- `render.drawSprite(sprite, Rect)`: Draws a sprite into a destination rectangle.
- `render.drawNineSlice(NineSlice, Rect)`: Draws a 9-slice scaled sprite.
- `sprite.update(scale, dt)`: Updates sprite physics and state.
- `sprite.wadMovement(speed, jump, dt)`: Built-in WASD movement for platformers.
- `sprite.updateDragDrop()`: Enables mouse interaction for the sprite.

### Advanced Rendering
- `render.drawTail(Tail, point, color, thickness, fade)`: Draws a trailing path.
- `render.drawGrid(Rect, xCount, yCount, colorA, colorB)`: Draws a checkerboard grid.
- `render.drawGridLines(Rect, xCount, yCount, color, thickness)`: Draws grid lines.
- `render.drawAxis(...)`: Draws a labeled coordinate axis.
- `render.drawParticles(ParticleEmitter)`: Draws all particles from an emitter.

### Text
- `render.drawText(string, pos, color, size)`: Draws a line of text.
- `render.drawList(vector<string>, pos, color, size)`: Draws a vertical list of strings.

---

## Input (Input.hpp)

Methods are static and accessed via `Input::`.

### Keyboard
- `isKeyDown(Key)`: True while key is held.
- `isKeyPressed(Key)`: True on the frame the key is first pressed.
- `isKeyReleased(Key)`: True on the frame the key is released.
- `wasdMovement(pos, speed, dt)`: Updates a position vector based on WASD keys.

### Mouse
- `getMousePos()`: Returns current mouse coordinates as `float2`.
- `isMouseClicked()`: Returns true on left-click.
- `isMouseButtonPressed(MouseButton)`: Checks specific buttons (Left, Middle, Right).
- `isMouseHoveringRect(mousePos, Rect)`: Collision check for mouse and rectangle.

---

## UI System (T_Core.hpp, LayoutManager.hpp)

### Components
- **Button**: `Button b; b.text = "Label"; render.drawButton(b);`
- **Slider**: `Slider s; s.value = 0.5f; render.drawSlider(s);`
- **InputField**: `InputField f; render.drawInputField(f);`
- **ColorPicker**: `ColorPicker cp; render.drawColorPicker(cp);`

### Layout Management
Automatic positioning of UI elements within a container.
- `render.column(width, padding, spacing)`: Starts a vertical layout.
- `render.row(height, padding, spacing)`: Starts a horizontal layout.
- `render.space(amount)`: Inserts a gap in the current layout.
- `render.end()`: Closes the current layout block.

---

## Particle System (ParticleEmitter.hpp)

```cpp
ParticleEmitter emitter(pos, lifeTime, startColor, endColor, startSize, endSize);
emitter.emissionRate = 20.0f;
emitter.update(dt);
render.drawParticles(emitter);
```

---

## Physics and Collisions (Collider.hpp)

- `Collider collider;`: Represents a collision volume.
- `collider.rectToCollider(Rect)`: Sets collider to a rectangle.
- `collider.circleToCollider(Circle)`: Sets collider to a circle.
- `collider.spriteToCollider(Sprite)`: Generates collider from sprite bounds.
- `collider.checkCollision(otherCollider)`: Returns true if two colliders overlap.
- `Collider::makeScreenBorderCollider(app)`: Creates boundaries at window edges.

---

## Math and Utilities (Math.hpp, T_Core.hpp)

### Types
- `float2`, `float3`, `float4`: Vector types with arithmetic operators.
- `Rect`, `Circle`, `Triangle`, `Line`, `Shape`.
- `Color`: `Color::White`, `Color::Black`, `Color::BgDeep`, `Color::Accent`, etc.
  - `Color::withAlpha(color, alpha)`
  - `Color::mix(colorA, colorB, t)`
  - `Color::lighten(color, amount)`, `Color::darken(color, amount)`

### Math Functions
- `Math::clamp(min, max, value)`
- `Math::distance(a, b)`
- `Math::normalize(float2)`
- `Math::randInt(min, max)`, `Math::randFloatRange(min, max)`
- **Easing**: `Math::Ease::lerp`, `quadLerpIn`, `cubicLerpOut`, `expoLerpInOut`, etc.
- **BigNum**: Utilities for converting and handling arbitrarily large numbers.

---

## Data and Files

### File I/O (File.hpp, MwFile.hpp)
- `File file("path.txt")`: Basic file operations.
  - `file.read()`, `file.write(string)`, `file.append(string)`.
  - `file.exists()`, `file.remove()`, `file.size()`.
- `MwFile mwf("path.mw")`: Binary file format with magic number validation.
  - `mwf.writeAtByte(offset, bytes)`, `mwf.readBytes(offset, count)`.
  - `mwf.writeASCII(offset, string)`, `mwf.writeAddress(offset, int)`.

### MTF Data Format (Mtf.hpp)
Hierarchical data format similar to JSON/YAML for configuration.
- `MTF data; data.load("config.mtf");`
- `data.section("Player/Stats")->get("Health", "100")`: Accesses nested properties.
- `data.save("output.mtf")`: Serializes state to disk.

---

## Image Processing (ImageProcess.hpp)

- `ImageProcess::whiteNoise(w, h)`: Generates a white noise texture.
- `ImageProcess::fractalNoise(w, h, scale, octaves)`: Generates fractal/Perlin-like noise.
- `ImageProcess::worleyNoise(w, h, scale)`: Generates Worley/cellular noise.
- `ImageProcess::smoothenImage(RawImage, radius)`: Applies a blur filter.
- `ImageProcess::fillImage(RawImage, color)`: Sets all pixels to a specific color.

---

## Audio (Audio.hpp)

- `Audio sound(app, "file.mp3")`: Loads an audio file.
- `sound.play()`: Starts playback.
- `sound.pause()`, `sound.stop(fadeOutFrames)`.
- `sound.setGain(float)`: Sets volume (0.0 to 1.0).
- `sound.loopAmount(int)`: Sets number of loops (-1 for infinite).
- `sound.setPitch(float)`: Adjusts playback speed.
