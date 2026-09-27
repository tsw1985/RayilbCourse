/* =============================================================================
   LESSON 03 - Input, and why everything is multiplied by delta time
   -----------------------------------------------------------------------------
   THREE WAYS TO ASK ABOUT A KEY, and they are not interchangeable:

     IsKeyDown(KEY_D)      true on EVERY frame while the key is held.
                           Use it for continuous things: walking, holding a button.

     IsKeyPressed(KEY_D)   true on ONLY ONE frame, the one where it went down.
                           Use it for one-shot things: jump, shoot, open a menu.

     IsKeyReleased(KEY_D)  true on only one frame, when it comes back up.

   Using IsKeyDown where you meant IsKeyPressed is the classic bug: your menu
   scrolls 60 entries in one second because you held the key for one second.

   DELTA TIME. GetFrameTime() gives the seconds the previous frame took: about
   0.016 at 60 FPS, about 0.008 at 120 FPS. If you write

       x += 5;                 // 5 pixels PER FRAME

   your game runs twice as fast on a 120 Hz monitor. If instead you write

       x += 300*GetFrameTime(); // 300 pixels PER SECOND

   it moves the same on every machine. Rule: any speed you write is per SECOND,
   and you multiply it by delta time. No exceptions.

   Controls: WASD/arrows move, SPACE for a one-shot pulse, mouse click to teleport.
   ============================================================================= */

#include "raylib.h"

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(900, 520, "Lesson 03 - Input and delta time");
    SetTargetFPS(60);

    Vector2 goodPlayer = { 200, 260 };   // moved with delta time
    Vector2 badPlayer  = { 200, 360 };   // moved per frame (wrong)

    int pulses = 0;          // counts IsKeyPressed hits
    int heldFrames = 0;      // counts IsKeyDown hits

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();      // seconds since the previous frame

        // --- Continuous movement: IsKeyDown ---------------------------------
        float speed = 300.0f;           // PIXELS PER SECOND

        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) { goodPlayer.x += speed*dt; badPlayer.x += 5; }
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  { goodPlayer.x -= speed*dt; badPlayer.x -= 5; }
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  { goodPlayer.y += speed*dt; badPlayer.y += 5; }
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    { goodPlayer.y -= speed*dt; badPlayer.y -= 5; }

        // --- One-shot action: IsKeyPressed -----------------------------------
        if (IsKeyPressed(KEY_SPACE)) pulses++;
        if (IsKeyDown(KEY_SPACE))    heldFrames++;

        // --- The mouse --------------------------------------------------------
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) goodPlayer = mouse;

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            DrawText("Both squares get the same key presses.", 40, 30, 20, DARKGRAY);
            DrawText("The green one uses delta time. The red one does not.", 40, 56, 16, GRAY);

            DrawRectangleV((Vector2){ goodPlayer.x - 20, goodPlayer.y - 20 }, (Vector2){ 40, 40 }, DARKGREEN);
            DrawText("300 px/second", (int)goodPlayer.x + 28, (int)goodPlayer.y - 8, 14, DARKGREEN);

            DrawRectangleV((Vector2){ badPlayer.x - 20, badPlayer.y - 20 }, (Vector2){ 40, 40 }, MAROON);
            DrawText("5 px/frame", (int)badPlayer.x + 28, (int)badPlayer.y - 8, 14, MAROON);

            // The mouse cursor position, in screen pixels
            DrawCircleLines((int)mouse.x, (int)mouse.y, 12, BLUE);
            DrawText(TextFormat("mouse (%.0f, %.0f)", mouse.x, mouse.y), (int)mouse.x + 16, (int)mouse.y, 14, BLUE);

            DrawRectangle(40, 400, 460, 96, Fade(BLACK, 0.06f));
            DrawText(TextFormat("IsKeyPressed(SPACE) fired %i times", pulses), 56, 414, 18, DARKGRAY);
            DrawText(TextFormat("IsKeyDown(SPACE)    fired %i times", heldFrames), 56, 440, 18, DARKGRAY);
            DrawText("hold SPACE for a second and compare the two", 56, 468, 14, GRAY);

            DrawText(TextFormat("dt = %.4f s   (%i FPS)", dt, GetFPS()), 620, 414, 18, DARKGRAY);
            DrawText("left click: teleport the green square", 620, 440, 14, GRAY);

            DrawFPS(800, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
