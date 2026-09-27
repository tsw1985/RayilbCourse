/* =============================================================================
   LESSON 03 - The camera modes raylib already ships with
   -----------------------------------------------------------------------------
   Before writing your own camera, try the ready-made ones:

     UpdateCamera(&camera, MODE);

   Available modes:
     CAMERA_FREE         : free flight. WASD + mouse + wheel. The camera orbits
                           around the target and can move closer/further away.
     CAMERA_ORBITAL      : spins by itself around the target. Great to inspect a model.
     CAMERA_FIRST_PERSON : first person. The target is recomputed from the mouse;
                           position is "your head".
     CAMERA_THIRD_PERSON : the camera follows the target from behind.

   Important: in FIRST_PERSON and THIRD_PERSON you need DisableCursor() so the
   mouse gets captured, otherwise rotation behaves oddly.

   Controls: TAB cycles modes. ESC quits.
   ============================================================================= */

#include "raylib.h"

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1000, 600, "Lesson 03 - Camera modes");

    Camera3D camera = { 0 };
    camera.position   = (Vector3){ 8.0f, 4.0f, 8.0f };
    camera.target     = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int modes[] = { CAMERA_FREE, CAMERA_ORBITAL, CAMERA_FIRST_PERSON, CAMERA_THIRD_PERSON };
    const char *names[] = { "CAMERA_FREE", "CAMERA_ORBITAL", "CAMERA_FIRST_PERSON", "CAMERA_THIRD_PERSON" };
    int current = 0;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_TAB))
        {
            current = (current + 1) % 4;

            // First/third person modes want the mouse captured
            if (modes[current] == CAMERA_FIRST_PERSON || modes[current] == CAMERA_THIRD_PERSON)
            {
                camera.position = (Vector3){ 0.0f, 2.0f, 6.0f };
                camera.target   = (Vector3){ 0.0f, 2.0f, 0.0f };
                camera.up       = (Vector3){ 0.0f, 1.0f, 0.0f };
                DisableCursor();
            }
            else
            {
                EnableCursor();
            }
        }

        // This single line does all the work of moving/turning the camera.
        UpdateCamera(&camera, modes[current]);

        BeginDrawing();
            ClearBackground((Color){ 24, 26, 32, 255 });

            BeginMode3D(camera);
                DrawGrid(20, 1.0f);

                // Reference scene: a few boxes scattered around
                DrawCube((Vector3){ 0, 1, 0 }, 2, 2, 2, MAROON);
                DrawCubeWires((Vector3){ 0, 1, 0 }, 2, 2, 2, RAYWHITE);
                DrawCube((Vector3){ -5, 1, -3 }, 2, 2, 2, DARKGREEN);
                DrawCube((Vector3){  5, 1,  3 }, 2, 2, 2, DARKBLUE);
                DrawCube((Vector3){ -4, 1,  5 }, 2, 2, 2, DARKPURPLE);

                // The axes, so you never lose your bearings
                DrawLine3D((Vector3){0,0,0}, (Vector3){6,0,0}, RED);
                DrawLine3D((Vector3){0,0,0}, (Vector3){0,6,0}, GREEN);
                DrawLine3D((Vector3){0,0,0}, (Vector3){0,0,6}, BLUE);
            EndMode3D();

            DrawRectangle(10, 10, 440, 120, Fade(BLACK, 0.65f));
            DrawText(TextFormat("MODE: %s", names[current]), 20, 18, 20, YELLOW);
            DrawText("TAB to cycle modes", 20, 46, 16, RAYWHITE);
            DrawText(TextFormat("position (%5.2f, %5.2f, %5.2f)", camera.position.x, camera.position.y, camera.position.z), 20, 70, 16, GRAY);
            DrawText(TextFormat("target   (%5.2f, %5.2f, %5.2f)", camera.target.x, camera.target.y, camera.target.z), 20, 92, 16, GRAY);
            DrawText(TextFormat("mouse %s", IsCursorHidden() ? "captured" : "free"), 20, 112, 14, GRAY);

            DrawFPS(GetScreenWidth() - 90, 10);
        EndDrawing();
    }

    EnableCursor();
    CloseWindow();
    return 0;
}
