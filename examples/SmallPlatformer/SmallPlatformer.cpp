//
// Created by Stěpán Toman on 12.05.2026.
//

#include <Translucence.hpp>

int main () {
    Application app;
    app.create(800, 600, "Smallscale platformer Example");
    app.setResizable(true);
    Renderer renderer(app);
    EventSystem events(app);

    Sprite player(app, "../../../examples/SmallPlatformer/assets/guy.png"); // I know this is very gross, but I don't really care lol
    player.pos = {0, 0};
    player.autoUpdatePhysics = true;
    player.useGravity = true;
    player.assignCollider();

    Rect ground = {0, app.getHeight() - 50.0f, cast<float>(app.getWidth()), 50};
    Rect obstacle1 = {app.getWidth() / 8.0f, app.getHeight() - (app.getHeight() / 3.0f), 150, 50};
    Rect obstacle2 = {app.getWidth() / 2.4f, app.getHeight() - (app.getHeight() / 2.0f), 150, 50};
    Rect obstacle3 = {app.getWidth() / 1.4f, app.getHeight() - (app.getHeight() / 1.5f), 150, 50};

    Collider gCollider;
    gCollider.rectToCollider(ground);
    Collider o1Collider;
    o1Collider.rectToCollider(obstacle1);
    Collider o2Collider;
    o2Collider.rectToCollider(obstacle2);
    Collider o3Collider;
    o3Collider.rectToCollider(obstacle3);

    Collider::makeScreenBorderCollider(app);

    while (app.isRunning()) {
        app.update();
        events.runEvents();
        renderer.clearBackground(Color::White);
        float dt =  app.getDeltaTime();
        if (Input::isKeyDown(Input::Key::ESCAPE)) app.setRunning(false);

        renderer.drawRect(ground, Color::DarkGray);
        renderer.drawRect(obstacle1, Color::SlateGray);
        renderer.drawRect(obstacle2, Color::SlateGray);
        renderer.drawRect(obstacle3, Color::SlateGray);

        player.wadMovement(300, 600, dt);
        player.update(2.5, dt);

        Collider::debug_drawColliders(app.getRenderer(), Color::Red);

        renderer.drawSprite(player, 2.5);
        renderer.drawText(toString(app.getFPS()), {0, 0}, Color::Crimson, 20);

        renderer.render();
    }
    return 0;
}