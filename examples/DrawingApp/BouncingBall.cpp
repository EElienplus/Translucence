#include <Translucence.hpp>

int main() {
    Application app;
    app.create(800, 600, "Bouncing Ball Example");
    Renderer renderer(app);
    EventSystem events(app);

    float2 ballPos = {400, 300};
    float2 ballVel = {200, 150};
    float ballRadius = 20.0f;

    while (app.isRunning()) {
        app.update();
        float dt = app.getDeltaTime();

        events.runEvents();
        if (Input::isKeyDown(Input::Key::ESCAPE)) app.setRunning(false);

        // Update physics
        ballPos.x += ballVel.x * dt;
        ballPos.y += ballVel.y * dt;

        // Bounce off walls
        if (ballPos.x - ballRadius < 0) {
            ballPos.x = ballRadius;
            ballVel.x *= -1;
        }
        if (ballPos.x + ballRadius > 800) {
            ballPos.x = 800 - ballRadius;
            ballVel.x *= -1;
        }
        if (ballPos.y - ballRadius < 0) {
            ballPos.y = ballRadius;
            ballVel.y *= -1;
        }
        if (ballPos.y + ballRadius > 600) {
            ballPos.y = 600 - ballRadius;
            ballVel.y *= -1;
        }

        renderer.clearBackground(Color::BgDeep);
        
        Circle ball = { ballPos, ballRadius };
        renderer.drawCircle(ball, Color::Accent);
        renderer.drawCircleOutline(ball, Color::White, 2);

        renderer.render();
    }

    return 0;
}
