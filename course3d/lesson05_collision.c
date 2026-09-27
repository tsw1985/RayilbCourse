/* =============================================================================
   LESSON 05 - The box that notices when you touch it
   -----------------------------------------------------------------------------
   The simplest form of 3D collision is the AABB
   (Axis-Aligned Bounding Box = a box aligned with the axes, that never rotates).

   In raylib it is this struct:

       typedef struct BoundingBox {
           Vector3 min;   // corner with the smaller coordinates (bottom-left-back)
           Vector3 max;   // corner with the larger coordinates (top-right-front)
       } BoundingBox;

   And the check is:

       bool CheckCollisionBoxes(BoundingBox a, BoundingBox b);

   Inside, it is trivial: two boxes collide if they overlap on ALL THREE axes at
   once. A single axis where they do not overlap means no collision.

       overlapX = (a.min.x <= b.max.x) && (a.max.x >= b.min.x)
       overlapY = (a.min.y <= b.max.y) && (a.max.y >= b.min.y)
       overlapZ = (a.min.z <= b.max.z) && (a.max.z >= b.min.z)
       hit      = overlapX && overlapY && overlapZ

   In this lesson you move a blue cube with WASD (and Q/E for height) and the
   orange box turns RED when you touch it. At the bottom you see, axis by axis,
   what is going on.

   Controls: WASD move on the XZ plane, Q/E up and down, R reset,
             mouse wheel to zoom the camera in and out.
   ============================================================================= */

#include "raylib.h"
#include "raymath.h"

// Builds a BoundingBox from a center and a size. This is the operation you will
// repeat the most: store your object's center and size, then compute the box
// every frame.
static BoundingBox BoxFromCenter(Vector3 center, Vector3 size)
{
    BoundingBox b;
    b.min = (Vector3){ center.x - size.x/2, center.y - size.y/2, center.z - size.z/2 };
    b.max = (Vector3){ center.x + size.x/2, center.y + size.y/2, center.z + size.z/2 };
    return b;
}

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1000, 620, "Lesson 05 - Colliding with a box");

    Camera3D camera = { 0 };
    camera.position   = (Vector3){ 10.0f, 9.0f, 10.0f };
    camera.target     = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // --- The player ---------------------------------------------------------
    Vector3 playerPos  = { -4.0f, 0.5f, 3.0f };
    Vector3 playerSize = { 1.0f, 1.0f, 1.0f };

    // --- The static box -----------------------------------------------------
    Vector3 boxPos  = { 0.0f, 1.0f, 0.0f };
    Vector3 boxSize = { 3.0f, 2.0f, 2.0f };   // width X=3, height Y=2, length Z=2

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        float v  = 4.0f*dt;

        // Movement along WORLD axes, not camera axes: that way it is obvious
        // which key moves along which axis.
        if (IsKeyDown(KEY_D)) playerPos.x += v;    // +X
        if (IsKeyDown(KEY_A)) playerPos.x -= v;    // -X
        if (IsKeyDown(KEY_S)) playerPos.z += v;    // +Z
        if (IsKeyDown(KEY_W)) playerPos.z -= v;    // -Z
        if (IsKeyDown(KEY_E)) playerPos.y += v;    // +Y
        if (IsKeyDown(KEY_Q)) playerPos.y -= v;    // -Y

        if (IsKeyPressed(KEY_R)) playerPos = (Vector3){ -4.0f, 0.5f, 3.0f };

        camera.position = Vector3Add(camera.position,
            Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)),
                         GetMouseWheelMove()*1.5f));

        // --- THE COLLISION ---------------------------------------------------
        BoundingBox playerBox = BoxFromCenter(playerPos, playerSize);
        BoundingBox staticBox = BoxFromCenter(boxPos, boxSize);

        bool hit = CheckCollisionBoxes(playerBox, staticBox);

        // The same thing, broken down axis by axis, just to show it on screen:
        bool overlapX = (playerBox.min.x <= staticBox.max.x) && (playerBox.max.x >= staticBox.min.x);
        bool overlapY = (playerBox.min.y <= staticBox.max.y) && (playerBox.max.y >= staticBox.min.y);
        bool overlapZ = (playerBox.min.z <= staticBox.max.z) && (playerBox.max.z >= staticBox.min.z);

        BeginDrawing();
            ClearBackground((Color){ 20, 22, 28, 255 });

            BeginMode3D(camera);
                DrawGrid(20, 1.0f);

                // Reference axes
                DrawLine3D((Vector3){0,0.02f,0}, (Vector3){8,0.02f,0}, RED);
                DrawLine3D((Vector3){0,0,0},     (Vector3){0,8,0},     GREEN);
                DrawLine3D((Vector3){0,0.02f,0}, (Vector3){0,0.02f,8}, BLUE);

                // The static box: changes color when touched
                Color boxColor = hit ? RED : ORANGE;
                DrawCubeV(boxPos, boxSize, Fade(boxColor, 0.7f));
                DrawCubeWiresV(boxPos, boxSize, boxColor);

                // The player
                DrawCubeV(playerPos, playerSize, Fade(SKYBLUE, 0.8f));
                DrawCubeWiresV(playerPos, playerSize, RAYWHITE);

                // Drawing the BoundingBoxes helps A LOT when debugging collisions
                DrawBoundingBox(playerBox, GREEN);
                DrawBoundingBox(staticBox, hit ? RED : GREEN);

                // Fake shadow, to see where the player is on the floor
                DrawCube((Vector3){ playerPos.x, 0.01f, playerPos.z }, 1.0f, 0.02f, 1.0f, Fade(BLACK, 0.5f));
            EndMode3D();

            // --- HUD ---------------------------------------------------------
            DrawRectangle(10, 10, 470, 210, Fade(BLACK, 0.7f));
            DrawText("AABB COLLISION", 20, 18, 20, WHITE);
            DrawText(TextFormat("player   X %6.2f   Y %6.2f   Z %6.2f", playerPos.x, playerPos.y, playerPos.z), 20, 48, 16, SKYBLUE);
            DrawText(TextFormat("box min (%5.2f, %5.2f, %5.2f)", staticBox.min.x, staticBox.min.y, staticBox.min.z), 20, 72, 16, ORANGE);
            DrawText(TextFormat("box max (%5.2f, %5.2f, %5.2f)", staticBox.max.x, staticBox.max.y, staticBox.max.z), 20, 92, 16, ORANGE);

            DrawText("they overlap on...", 20, 120, 16, GRAY);
            DrawText(TextFormat("X: %s", overlapX ? "YES" : "no"), 175, 120, 16, overlapX ? GREEN : GRAY);
            DrawText(TextFormat("Y: %s", overlapY ? "YES" : "no"), 255, 120, 16, overlapY ? GREEN : GRAY);
            DrawText(TextFormat("Z: %s", overlapZ ? "YES" : "no"), 335, 120, 16, overlapZ ? GREEN : GRAY);
            DrawText("all THREE at once -> there is a hit", 20, 142, 14, GRAY);

            DrawText("WASD: X/Z    Q/E: height    R: reset", 20, 172, 14, GRAY);
            DrawText("mouse wheel: zoom camera", 20, 192, 14, GRAY);

            if (hit)
            {
                const char *txt = "COLLISION!";
                int width = MeasureText(txt, 50);
                DrawRectangle(GetScreenWidth()/2 - width/2 - 20, 30, width + 40, 70, Fade(RED, 0.35f));
                DrawText(txt, GetScreenWidth()/2 - width/2, 45, 50, RED);
            }

            DrawFPS(GetScreenWidth() - 90, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
