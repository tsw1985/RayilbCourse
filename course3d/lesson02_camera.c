/* =============================================================================
   LESSON 02 - Anatomy of a camera
   -----------------------------------------------------------------------------
   A Camera3D is not magic, it is 5 fields:

     camera.position    Vector3  -> where the eye is
     camera.target      Vector3  -> which point of the world it looks at
     camera.up          Vector3  -> which direction it considers "up"
     camera.fovy        float    -> vertical field of view, in DEGREES
     camera.projection  int      -> CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC

   The direction it looks in is NOT stored: it is derived.
        direction = normalize(target - position)
   That is why, to turn the camera, you move the TARGET. To move it around, you
   move position (and usually the target by the same amount, so it does not turn).

   Controls:
     ARROWS / PgUp-PgDn : move position (X, Z, Y)
     I J K L / U O      : move target   (Z, X, Y)
     + / -              : fovy
     P                  : perspective <-> orthographic
     R                  : reset
   ============================================================================= */

#include "raylib.h"
#include "raymath.h"   // Vector3Add, Vector3Subtract, Vector3Normalize...

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1000, 600, "Lesson 02 - Anatomy of a camera");

    Camera3D camera = { 0 };
    camera.position   = (Vector3){ 6.0f, 5.0f, 6.0f };
    camera.target     = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();          // seconds since the previous frame
        float v  = 5.0f*dt;                 // speed: units per second

        // --- move the EYE ----------------------------------------------------
        if (IsKeyDown(KEY_RIGHT))     camera.position.x += v;
        if (IsKeyDown(KEY_LEFT))      camera.position.x -= v;
        if (IsKeyDown(KEY_DOWN))      camera.position.z += v;
        if (IsKeyDown(KEY_UP))        camera.position.z -= v;
        if (IsKeyDown(KEY_PAGE_UP))   camera.position.y += v;
        if (IsKeyDown(KEY_PAGE_DOWN)) camera.position.y -= v;

        // --- move the POINT IT LOOKS AT --------------------------------------
        if (IsKeyDown(KEY_L)) camera.target.x += v;
        if (IsKeyDown(KEY_J)) camera.target.x -= v;
        if (IsKeyDown(KEY_K)) camera.target.z += v;
        if (IsKeyDown(KEY_I)) camera.target.z -= v;
        if (IsKeyDown(KEY_U)) camera.target.y += v;
        if (IsKeyDown(KEY_O)) camera.target.y -= v;

        // --- the lens --------------------------------------------------------
        // In perspective, fovy is degrees (10 = telephoto, 90 = fisheye).
        // In orthographic, fovy becomes the world HEIGHT that fits on screen.
        if (IsKeyDown(KEY_EQUAL) || IsKeyDown(KEY_KP_ADD))      camera.fovy += 30.0f*dt;
        if (IsKeyDown(KEY_MINUS) || IsKeyDown(KEY_KP_SUBTRACT)) camera.fovy -= 30.0f*dt;
        if (camera.fovy < 5.0f)   camera.fovy = 5.0f;
        if (camera.fovy > 120.0f) camera.fovy = 120.0f;

        if (IsKeyPressed(KEY_P))
        {
            if (camera.projection == CAMERA_PERSPECTIVE)
            {
                camera.projection = CAMERA_ORTHOGRAPHIC;
                camera.fovy = 12.0f;   // now it means "12 units tall"
            }
            else
            {
                camera.projection = CAMERA_PERSPECTIVE;
                camera.fovy = 45.0f;   // now it means "45 degrees" again
            }
        }

        if (IsKeyPressed(KEY_R))
        {
            camera.position = (Vector3){ 6.0f, 5.0f, 6.0f };
            camera.target   = (Vector3){ 0.0f, 1.0f, 0.0f };
            camera.fovy     = 45.0f;
            camera.projection = CAMERA_PERSPECTIVE;
        }

        // The view direction is DERIVED data, not a field of the camera:
        Vector3 direction = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        float   distance  = Vector3Distance(camera.position, camera.target);

        BeginDrawing();
            ClearBackground((Color){ 24, 26, 32, 255 });

            BeginMode3D(camera);
                DrawGrid(20, 1.0f);

                // Minimal scene, just to have visual references
                DrawCube((Vector3){ 0, 1, 0 }, 2, 2, 2, MAROON);
                DrawCubeWires((Vector3){ 0, 1, 0 }, 2, 2, 2, RAYWHITE);
                DrawCube((Vector3){ -4, 0.5f, 2 }, 1, 1, 1, DARKGREEN);
                DrawCube((Vector3){ 3, 1.5f, -3 }, 1, 3, 1, DARKBLUE);

                // The TARGET marked with a yellow sphere: the point it looks at.
                DrawSphere(camera.target, 0.2f, YELLOW);

                // And the "up" vector coming out of the target
                DrawLine3D(camera.target,
                           Vector3Add(camera.target, Vector3Scale(camera.up, 1.5f)),
                           SKYBLUE);
            EndMode3D();

            DrawRectangle(10, 10, 430, 190, Fade(BLACK, 0.65f));
            DrawText("A CAMERA IS 5 PIECES OF DATA", 20, 18, 20, WHITE);
            DrawText(TextFormat("position   (%5.2f, %5.2f, %5.2f)", camera.position.x, camera.position.y, camera.position.z), 20, 48, 16, RAYWHITE);
            DrawText(TextFormat("target     (%5.2f, %5.2f, %5.2f)  <- yellow sphere", camera.target.x, camera.target.y, camera.target.z), 20, 70, 16, YELLOW);
            DrawText(TextFormat("up         (%5.2f, %5.2f, %5.2f)", camera.up.x, camera.up.y, camera.up.z), 20, 92, 16, SKYBLUE);
            DrawText(TextFormat("fovy       %5.1f  %s", camera.fovy,
                                camera.projection == CAMERA_PERSPECTIVE ? "degrees (PERSPECTIVE)" : "units (ORTHOGRAPHIC)"), 20, 114, 16, ORANGE);
            DrawText(TextFormat("direction  (%5.2f, %5.2f, %5.2f)   dist %.2f", direction.x, direction.y, direction.z, distance), 20, 136, 16, GRAY);
            DrawText("ARROWS+PgUp/PgDn: eye   IJKL+U/O: target", 20, 160, 14, GRAY);
            DrawText("+/-: fovy    P: projection    R: reset", 20, 178, 14, GRAY);

            DrawFPS(GetScreenWidth() - 90, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
