/* =============================================================================
   LESSON 04 - Your own first person camera (hand made)
   -----------------------------------------------------------------------------
   UpdateCamera() is fine, but if you want real control over your game you have
   to know how to do it yourself. All it takes is two angles:

     yaw   : horizontal turn, around the Y axis   (look left/right)
     pitch : vertical turn,   around the X axis   (look up/down)

   From those two angles you get the "forward" vector:

     forward.x = cos(pitch) * cos(yaw)
     forward.y = sin(pitch)
     forward.z = cos(pitch) * sin(yaw)

   And the "right" vector is the cross product of forward with world up:

     right = normalize( cross(forward, (0,1,0)) )

   With those two you can already move: W adds forward, D adds right, and so on.
   Finally:  camera.target = camera.position + forward.

   NOTE about pitch: you must clamp it to a bit less than 90 degrees. If you look
   straight up, "forward" lines up with "up", the cross product becomes zero and
   the camera snaps. That is the well known "gimbal lock".

   Controls: mouse to look, WASD to move, SHIFT to run, SPACE/CTRL to go up-down,
             TAB to release the mouse.
   ============================================================================= */

#include "raylib.h"
#include "raymath.h"
#include <math.h>

#define EYE_HEIGHT   1.7f     // a person 1.70 m tall

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1000, 600, "Lesson 04 - Hand made first person camera");

    Camera3D camera = { 0 };
    camera.position   = (Vector3){ 0.0f, EYE_HEIGHT, 6.0f };
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Angles in RADIANS. yaw = -PI/2 means "looking towards -Z".
    float yaw   = -PI/2.0f;
    float pitch = 0.0f;

    const float sensitivity = 0.003f;   // radians per mouse pixel
    const float pitchLimit  = 1.55f;    // ~89 degrees

    DisableCursor();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_TAB))
        {
            if (IsCursorHidden()) EnableCursor(); else DisableCursor();
        }

        // --- 1) LOOK: the mouse changes the angles ---------------------------
        if (IsCursorHidden())
        {
            Vector2 mouse = GetMouseDelta();      // pixels moved this frame
            yaw   += mouse.x*sensitivity;
            pitch -= mouse.y*sensitivity;         // subtract: mouse down = look down

            if (pitch >  pitchLimit) pitch =  pitchLimit;
            if (pitch < -pitchLimit) pitch = -pitchLimit;
        }

        // --- 2) From the angles to the vectors -------------------------------
        Vector3 forward = {
            cosf(pitch)*cosf(yaw),
            sinf(pitch),
            cosf(pitch)*sinf(yaw)
        };

        Vector3 right = Vector3Normalize(
            Vector3CrossProduct(forward, (Vector3){ 0.0f, 1.0f, 0.0f }));

        // To WALK on the ground we want a flat "forward": no Y component.
        // Otherwise, looking at the sky and pressing W would launch you upwards.
        Vector3 flatForward = Vector3Normalize((Vector3){ forward.x, 0.0f, forward.z });

        // --- 3) MOVE ---------------------------------------------------------
        float speed = IsKeyDown(KEY_LEFT_SHIFT) ? 10.0f : 4.0f;   // m/s
        Vector3 movement = { 0 };

        if (IsKeyDown(KEY_W)) movement = Vector3Add(movement, flatForward);
        if (IsKeyDown(KEY_S)) movement = Vector3Subtract(movement, flatForward);
        if (IsKeyDown(KEY_D)) movement = Vector3Add(movement, right);
        if (IsKeyDown(KEY_A)) movement = Vector3Subtract(movement, right);

        // Normalizing stops diagonal movement (W+D) from being faster.
        if (Vector3Length(movement) > 0.0f)
            movement = Vector3Normalize(movement);

        // Multiplying by dt makes the speed independent of the frame rate.
        camera.position = Vector3Add(camera.position, Vector3Scale(movement, speed*dt));

        // Go up and down vertically ("fly" mode)
        if (IsKeyDown(KEY_SPACE))        camera.position.y += speed*dt;
        if (IsKeyDown(KEY_LEFT_CONTROL)) camera.position.y -= speed*dt;
        if (camera.position.y < 0.2f)    camera.position.y = 0.2f;   // do not go through the floor

        // --- 4) And this is what makes the camera look the right way ---------
        camera.target = Vector3Add(camera.position, forward);

        BeginDrawing();
            ClearBackground((Color){ 18, 20, 26, 255 });

            BeginMode3D(camera);
                DrawGrid(40, 1.0f);
                DrawPlane((Vector3){ 0, -0.01f, 0 }, (Vector2){ 40, 40 }, (Color){ 32, 36, 44, 255 });

                // Colored posts so you can orient yourself in the scene
                DrawCube((Vector3){  10, 1.5f,   0 }, 1, 3, 1, RED);        // towards +X
                DrawCube((Vector3){ -10, 1.5f,   0 }, 1, 3, 1, MAROON);     // towards -X
                DrawCube((Vector3){   0, 1.5f,  10 }, 1, 3, 1, BLUE);       // towards +Z
                DrawCube((Vector3){   0, 1.5f, -10 }, 1, 3, 1, DARKBLUE);   // towards -Z
                DrawCube((Vector3){   0, 4.0f,   0 }, 1, 1, 1, GREEN);      // up, +Y

                DrawCube((Vector3){ 3, 0.5f, -4 }, 1, 1, 1, ORANGE);
                DrawCube((Vector3){ -4, 0.5f, 3 }, 1, 1, 1, PURPLE);
            EndMode3D();

            // Crosshair
            DrawCircleLines(GetScreenWidth()/2, GetScreenHeight()/2, 4, RAYWHITE);

            DrawRectangle(10, 10, 430, 150, Fade(BLACK, 0.65f));
            DrawText("HAND MADE CAMERA", 20, 18, 20, WHITE);
            DrawText(TextFormat("yaw   %7.1f deg", yaw*RAD2DEG), 20, 46, 16, ORANGE);
            DrawText(TextFormat("pitch %7.1f deg", pitch*RAD2DEG), 20, 68, 16, ORANGE);
            DrawText(TextFormat("position  X %6.2f   Y %6.2f   Z %6.2f", camera.position.x, camera.position.y, camera.position.z), 20, 90, 16, RAYWHITE);
            DrawText(TextFormat("forward   X %6.2f   Y %6.2f   Z %6.2f", forward.x, forward.y, forward.z), 20, 112, 16, SKYBLUE);
            DrawText("WASD move  mouse look  SHIFT run  TAB mouse", 20, 138, 14, GRAY);

            DrawFPS(GetScreenWidth() - 90, 10);
        EndDrawing();
    }

    EnableCursor();
    CloseWindow();
    return 0;
}
