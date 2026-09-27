/* =============================================================================
   LESSON 06 - Camera2D: the bridge to the 3D camera
   -----------------------------------------------------------------------------
   So far every coordinate we drew was a SCREEN pixel. Real games do not work
   like that: the world is bigger than the window, so you store WORLD
   coordinates and let a camera decide what part of the world is on screen.

   Camera2D has four fields:

       camera.target    Vector2  the world point the camera is centred on
       camera.offset    Vector2  where that point lands ON SCREEN
                                 (usually the middle of the window)
       camera.rotation  float    degrees, rotates the whole world
       camera.zoom      float    1.0 = normal, 2.0 = twice as big

   And it is switched on and off exactly like the 3D one:

       BeginMode2D(camera);
           // coordinates here are WORLD coordinates
       EndMode2D();
       // coordinates here are SCREEN pixels again: this is where the HUD goes

   That Begin/End pair is the single most important habit in raylib. Anything
   drawn inside is transformed by the camera; anything outside is not. The HUD
   must not move with the world, so the HUD lives outside. Same rule, same
   reasoning, in BeginMode3D.

   Two helpers translate between the two worlds:
       GetWorldToScreen2D(worldPoint, camera)  ->  where it is on screen
       GetScreenToWorld2D(screenPoint, camera) ->  what the mouse is pointing at

   Controls: WASD move, mouse wheel zoom, Q/E rotate, R reset.
   ============================================================================= */

#include "raylib.h"

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(900, 520, "Lesson 06 - Camera2D");
    SetTargetFPS(60);

    Vector2 player = { 0, 0 };   // WORLD coordinates, the world is centred on 0,0

    Camera2D camera = { 0 };
    camera.target   = player;
    camera.offset   = (Vector2){ 450, 260 };   // the centre of the window
    camera.rotation = 0.0f;
    camera.zoom     = 1.0f;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float speed = 300.0f;

        if (IsKeyDown(KEY_D)) player.x += speed*dt;
        if (IsKeyDown(KEY_A)) player.x -= speed*dt;
        if (IsKeyDown(KEY_S)) player.y += speed*dt;
        if (IsKeyDown(KEY_W)) player.y -= speed*dt;

        if (IsKeyDown(KEY_Q)) camera.rotation -= 40.0f*dt;
        if (IsKeyDown(KEY_E)) camera.rotation += 40.0f*dt;

        camera.zoom += GetMouseWheelMove()*0.1f;
        if (camera.zoom < 0.2f) camera.zoom = 0.2f;
        if (camera.zoom > 4.0f) camera.zoom = 4.0f;

        if (IsKeyPressed(KEY_R)) { camera.rotation = 0.0f; camera.zoom = 1.0f; player = (Vector2){ 0, 0 }; }

        // The camera follows the player: its target IS the player position.
        camera.target = player;

        // What world point is the mouse hovering over?
        Vector2 mouseWorld = GetScreenToWorld2D(GetMousePosition(), camera);

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            // ================= WORLD =================
            BeginMode2D(camera);

                // A big world, much larger than the window
                for (int x = -1000; x <= 1000; x += 100)
                    DrawLine(x, -1000, x, 1000, (Color){ 225, 225, 225, 255 });
                for (int y = -1000; y <= 1000; y += 100)
                    DrawLine(-1000, y, 1000, y, (Color){ 225, 225, 225, 255 });

                // World origin and axes
                DrawLine(0, 0, 150, 0, RED);
                DrawLine(0, 0, 0, 150, GREEN);
                DrawCircle(0, 0, 6, BLACK);

                // Some landmarks scattered around the world
                DrawRectangle(-400, -300, 120, 120, MAROON);
                DrawRectangle( 350, -150,  90, 200, DARKGREEN);
                DrawRectangle(-200,  300, 260,  60, DARKPURPLE);
                DrawCircle(500, 400, 70, ORANGE);

                // The player, in world coordinates
                DrawRectangle((int)player.x - 15, (int)player.y - 15, 30, 30, DARKBLUE);

                // The point under the mouse, in world coordinates
                DrawCircleLines((int)mouseWorld.x, (int)mouseWorld.y, 10, BLUE);

            EndMode2D();
            // ================= SCREEN =================
            // Everything from here on is fixed to the window: the HUD.

            DrawRectangle(0, 0, 900, 70, Fade(BLACK, 0.75f));
            DrawText(TextFormat("player in WORLD  (%.0f, %.0f)", player.x, player.y), 20, 14, 18, RAYWHITE);
            DrawText(TextFormat("mouse in WORLD   (%.0f, %.0f)", mouseWorld.x, mouseWorld.y), 20, 40, 16, SKYBLUE);
            DrawText(TextFormat("zoom %.2f   rotation %.0f deg", camera.zoom, camera.rotation), 420, 14, 18, RAYWHITE);
            DrawText("WASD move   wheel zoom   Q/E rotate   R reset", 420, 40, 16, GRAY);

            // Proof that the HUD is not affected by the camera: this crosshair
            // stays nailed to the middle of the window whatever the camera does.
            DrawCircleLines(450, 260, 22, Fade(RED, 0.6f));
            DrawText("screen centre", 470, 250, 14, Fade(RED, 0.8f));

            DrawFPS(820, 90);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
