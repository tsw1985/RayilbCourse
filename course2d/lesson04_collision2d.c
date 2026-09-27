/* =============================================================================
   LESSON 04 - 2D collisions, the rehearsal for the 3D ones
   -----------------------------------------------------------------------------
   A Rectangle in raylib is:

       typedef struct Rectangle {
           float x;        // LEFT edge
           float y;        // TOP edge
           float width;
           float height;
       } Rectangle;

   Two rectangles overlap if they overlap on BOTH axes at once:

       overlapX = (a.x < b.x + b.width)  && (a.x + a.width  > b.x)
       overlapY = (a.y < b.y + b.height) && (a.y + a.height > b.y)
       hit      = overlapX && overlapY

   raylib gives you that as CheckCollisionRecs(a, b). Remember this shape: in 3D
   it is exactly the same idea with one extra axis (see course3d/lesson05).

   RESOLVING the collision (making the wall actually stop you) is done axis by
   axis: try X, undo if it hits; then try Y, undo if it hits. That is what lets
   you slide along a wall instead of sticking to it.

   Controls: WASD move, SPACE toggles between "detect only" and "walls stop you".
   ============================================================================= */

#include "raylib.h"

#define WALL_COUNT 5

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(900, 520, "Lesson 04 - 2D collisions");
    SetTargetFPS(60);

    Rectangle player = { 80, 80, 40, 40 };

    Rectangle walls[WALL_COUNT] = {
        { 300, 120, 300,  30 },
        { 300, 150,  30, 240 },
        { 560, 260, 260,  30 },
        { 150, 360, 250,  30 },
        { 640,  60,  30, 160 },
    };

    bool solid = true;   // do the walls stop the player?

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float speed = 240.0f;

        if (IsKeyPressed(KEY_SPACE)) solid = !solid;

        float dx = 0, dy = 0;
        if (IsKeyDown(KEY_D)) dx += speed*dt;
        if (IsKeyDown(KEY_A)) dx -= speed*dt;
        if (IsKeyDown(KEY_S)) dy += speed*dt;
        if (IsKeyDown(KEY_W)) dy -= speed*dt;

        // --- MOVE AND CHECK, ONE AXIS AT A TIME -----------------------------
        bool touching = false;

        // X axis
        Rectangle attempt = player;
        attempt.x += dx;
        bool blockedX = false;
        for (int i = 0; i < WALL_COUNT; i++)
            if (CheckCollisionRecs(attempt, walls[i])) { blockedX = true; touching = true; }

        if (!blockedX || !solid) player.x = attempt.x;

        // Y axis
        attempt = player;
        attempt.y += dy;
        bool blockedY = false;
        for (int i = 0; i < WALL_COUNT; i++)
            if (CheckCollisionRecs(attempt, walls[i])) { blockedY = true; touching = true; }

        if (!blockedY || !solid) player.y = attempt.y;

        // Keep the player inside the window
        if (player.x < 0) player.x = 0;
        if (player.y < 0) player.y = 0;
        if (player.x + player.width  > 900) player.x = 900 - player.width;
        if (player.y + player.height > 520) player.y = 520 - player.height;

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            for (int i = 0; i < WALL_COUNT; i++)
            {
                bool hit = CheckCollisionRecs(player, walls[i]);
                DrawRectangleRec(walls[i], hit ? RED : DARKGRAY);
            }

            DrawRectangleRec(player, touching ? ORANGE : DARKBLUE);
            DrawRectangleLinesEx(player, 2, BLACK);

            DrawRectangle(20, 20, 420, 92, Fade(BLACK, 0.06f));
            DrawText(TextFormat("player  x %.0f  y %.0f  w %.0f  h %.0f",
                     player.x, player.y, player.width, player.height), 34, 32, 18, DARKGRAY);
            DrawText(solid ? "walls: SOLID (movement resolved per axis)"
                           : "walls: GHOST (collision only detected)", 34, 58, 18, solid ? DARKGREEN : MAROON);
            DrawText("WASD move    SPACE toggle solid/ghost", 34, 84, 14, GRAY);

            DrawFPS(800, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
