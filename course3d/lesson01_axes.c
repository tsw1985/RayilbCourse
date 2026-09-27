/* =============================================================================
   LESSON 01 - The X, Y, Z axes
   -----------------------------------------------------------------------------
   Goal: see with your own eyes where each axis points.

   raylib uses a right-handed coordinate system, same as OpenGL:

        +Y  (up)
         |
         |
         +--------- +X  (to your right)
        /
       /
     +Z  (towards you, out of the screen)

   Rules worth memorizing:
     - X : left (-)   / right (+)
     - Y : down (-)   / up (+)        <-- Y is the HEIGHT
     - Z : far (-)    / near (+)      <-- Z is the DEPTH

   Notice: in 2D, Y grew DOWNWARDS (pixels). In 3D, Y grows UPWARDS.
   That is mistake number one for everybody starting out.

   Controls: 1,2,3,4 to jump between views. SPACE toggles the helper cubes.
   ============================================================================= */

#include "raylib.h"

// Draws the three axes with their standard colors:
//   X = RED, Y = GREEN, Z = BLUE   (mnemonic: R-G-B  ->  X-Y-Z)
static void DrawAxes(float length)
{
    // Positive half: solid, bright line
    DrawLine3D((Vector3){ 0, 0, 0 }, (Vector3){ length, 0, 0 }, RED);    // +X
    DrawLine3D((Vector3){ 0, 0, 0 }, (Vector3){ 0, length, 0 }, GREEN);  // +Y
    DrawLine3D((Vector3){ 0, 0, 0 }, (Vector3){ 0, 0, length }, BLUE);   // +Z

    // Negative half: dimmed, so you can tell them apart
    DrawLine3D((Vector3){ 0, 0, 0 }, (Vector3){ -length, 0, 0 }, Fade(RED, 0.3f));
    DrawLine3D((Vector3){ 0, 0, 0 }, (Vector3){ 0, -length, 0 }, Fade(GREEN, 0.3f));
    DrawLine3D((Vector3){ 0, 0, 0 }, (Vector3){ 0, 0, -length }, Fade(BLUE, 0.3f));

    // A small cube at the tip of each positive axis, to show its direction.
    // DrawCube(center, width(X), height(Y), length(Z), color)
    DrawCube((Vector3){ length, 0, 0 }, 0.4f, 0.4f, 0.4f, RED);
    DrawCube((Vector3){ 0, length, 0 }, 0.4f, 0.4f, 0.4f, GREEN);
    DrawCube((Vector3){ 0, 0, length }, 0.4f, 0.4f, 0.4f, BLUE);

    // The origin of the world: the point (0, 0, 0)
    DrawSphere((Vector3){ 0, 0, 0 }, 0.15f, WHITE);
}

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1000, 600, "Lesson 01 - The X, Y, Z axes");

    // --- THE CAMERA ----------------------------------------------------------
    // A 3D camera in raylib is just 5 pieces of data. Nothing else.
    Camera3D camera = { 0 };
    camera.position   = (Vector3){ 8.0f, 6.0f, 8.0f }; // WHERE the eye is
    camera.target     = (Vector3){ 0.0f, 0.0f, 0.0f }; // WHICH POINT it looks at
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f }; // WHAT "up" MEANS (almost always +Y)
    camera.fovy       = 45.0f;                         // Field of view, in degrees
    camera.projection = CAMERA_PERSPECTIVE;            // Perspective (far things look smaller)

    bool helpers = true;

    while (!WindowShouldClose())
    {
        // --- UPDATE ----------------------------------------------------------
        // We move the camera EYE to specific spots to understand the axes.
        // The target always stays at the origin (0,0,0).
        if (IsKeyPressed(KEY_ONE))   camera.position = (Vector3){  8.0f,  6.0f,  8.0f }; // free view
        if (IsKeyPressed(KEY_TWO))   camera.position = (Vector3){  0.0f,  0.0f, 12.0f }; // from +Z: I see the XY plane
        if (IsKeyPressed(KEY_THREE)) camera.position = (Vector3){  0.0f, 12.0f,  0.1f }; // from +Y (top down): I see the XZ plane
        if (IsKeyPressed(KEY_FOUR))  camera.position = (Vector3){ 12.0f,  0.0f,  0.0f }; // from +X: I see the ZY plane
        if (IsKeyPressed(KEY_SPACE)) helpers = !helpers;

        // --- DRAW ------------------------------------------------------------
        BeginDrawing();
            ClearBackground((Color){ 24, 26, 32, 255 });

            // EVERYTHING drawn between BeginMode3D and EndMode3D uses WORLD
            // coordinates (meters, say) and gets projected by the camera.
            BeginMode3D(camera);

                // Grid on the XZ plane (the "floor"): 20 cells of 1 unit each.
                // Careful: the grid sits at height Y = 0.
                DrawGrid(20, 1.0f);

                DrawAxes(5.0f);

                if (helpers)
                {
                    // Three cubes 3 units along each axis, for comparison.
                    DrawCube((Vector3){ 3, 0, 0 }, 1, 1, 1, Fade(RED, 0.5f));
                    DrawCubeWires((Vector3){ 3, 0, 0 }, 1, 1, 1, RED);

                    DrawCube((Vector3){ 0, 3, 0 }, 1, 1, 1, Fade(GREEN, 0.5f));
                    DrawCubeWires((Vector3){ 0, 3, 0 }, 1, 1, 1, GREEN);

                    DrawCube((Vector3){ 0, 0, 3 }, 1, 1, 1, Fade(BLUE, 0.5f));
                    DrawCubeWires((Vector3){ 0, 0, 3 }, 1, 1, 1, BLUE);
                }

            EndMode3D();

            // Outside Mode3D we are back to SCREEN coordinates (2D pixels).
            // Here Y does grow downwards. This is where the HUD and text go.
            DrawRectangle(10, 10, 420, 150, Fade(BLACK, 0.6f));
            DrawText("WORLD AXES", 20, 20, 20, WHITE);
            DrawText("X  ->  red     : left(-)  / right(+)", 20, 50, 16, RED);
            DrawText("Y  ->  green   : down(-)  / UP(+)", 20, 72, 16, GREEN);
            DrawText("Z  ->  blue    : far(-)   / towards you(+)", 20, 94, 16, BLUE);
            DrawText("1/2/3/4 change view   SPACE helpers", 20, 126, 16, GRAY);

            // Handy trick: project a 3D point onto the screen to label it.
            Vector2 pX = GetWorldToScreen((Vector3){ 5.4f, 0, 0 }, camera);
            Vector2 pY = GetWorldToScreen((Vector3){ 0, 5.4f, 0 }, camera);
            Vector2 pZ = GetWorldToScreen((Vector3){ 0, 0, 5.4f }, camera);
            DrawText("+X", (int)pX.x, (int)pX.y, 20, RED);
            DrawText("+Y", (int)pY.x, (int)pY.y, 20, GREEN);
            DrawText("+Z", (int)pZ.x, (int)pZ.y, 20, BLUE);

            DrawFPS(GetScreenWidth() - 90, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
