/* =============================================================================
   LESSON 06 - A full scene: walk, bump into things, and be stopped by walls
   -----------------------------------------------------------------------------
   We put everything together:
     - hand made first person camera      (lesson 04)
     - AABB collisions                    (lesson 05)
     - and one new thing: RESOLVING the collision, not just detecting it.

   THE TECHNIQUE: "move and check, one axis at a time".

     1. Work out how far you want to move this frame.
     2. Try moving ONLY along X. If you hit something, undo that X movement.
     3. Try moving ONLY along Z. If you hit something, undo that Z movement.

   Separating the axes is what lets you SLIDE along a wall instead of getting
   stuck. If you checked both axes at once, brushing a wall diagonally would
   stop you dead.

   Controls: WASD move, mouse look, SHIFT run, TAB release mouse, R reset.
   ============================================================================= */

#include "raylib.h"
#include "raymath.h"
#include <math.h>

#define MAX_BOXES     8
#define EYE_HEIGHT    1.7f
#define PLAYER_RADIUS 0.35f     // the player is a small box, 0.7 x 1.7 x 0.7

typedef struct Box {
    Vector3 position;    // center
    Vector3 size;        // width(X), height(Y), length(Z)
    Color   color;
    bool    touched;     // set to true on the frame the player touches it
} Box;

static BoundingBox BoxFromCenter(Vector3 center, Vector3 size)
{
    BoundingBox b;
    b.min = (Vector3){ center.x - size.x/2, center.y - size.y/2, center.z - size.z/2 };
    b.max = (Vector3){ center.x + size.x/2, center.y + size.y/2, center.z + size.z/2 };
    return b;
}

// Returns the index of the box the player would hit if standing at 'position',
// or -1 if it hits nothing.
static int CollisionIndex(Vector3 position, Box *boxes, int count)
{
    // The player: 'position' is the height of the EYES, so the center of the
    // body sits half a body lower.
    Vector3 bodyCenter = { position.x, position.y - EYE_HEIGHT/2.0f, position.z };
    Vector3 bodySize   = { PLAYER_RADIUS*2, EYE_HEIGHT, PLAYER_RADIUS*2 };

    BoundingBox body = BoxFromCenter(bodyCenter, bodySize);

    for (int i = 0; i < count; i++)
    {
        if (CheckCollisionBoxes(body, BoxFromCenter(boxes[i].position, boxes[i].size)))
            return i;
    }
    return -1;
}

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1000, 620, "Lesson 06 - Scene with collisions");

    Camera3D camera = { 0 };
    camera.position   = (Vector3){ 0.0f, EYE_HEIGHT, 8.0f };
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float yaw   = -PI/2.0f;
    float pitch = 0.0f;

    // --- The scene: four walls and a few blocks -----------------------------
    Box boxes[MAX_BOXES] = {
        { { 0.0f, 1.0f,  0.0f }, { 2.0f, 2.0f, 2.0f }, ORANGE,     false }, // the cube in the middle
        { { 4.0f, 0.5f, -3.0f }, { 1.0f, 1.0f, 1.0f }, PURPLE,     false },
        { {-4.0f, 1.5f,  2.0f }, { 1.0f, 3.0f, 1.0f }, DARKGREEN,  false },
        { { 2.0f, 0.5f,  5.0f }, { 3.0f, 1.0f, 1.0f }, GOLD,       false },
        { { 0.0f, 1.5f,-12.0f }, { 24.0f, 3.0f, 0.5f }, DARKGRAY,  false }, // north wall (-Z)
        { { 0.0f, 1.5f, 12.0f }, { 24.0f, 3.0f, 0.5f }, DARKGRAY,  false }, // south wall (+Z)
        { {-12.0f,1.5f,  0.0f }, { 0.5f, 3.0f, 24.0f }, DARKGRAY,  false }, // west wall  (-X)
        { { 12.0f,1.5f,  0.0f }, { 0.5f, 3.0f, 24.0f }, DARKGRAY,  false }, // east wall  (+X)
    };

    int touchedNow = -1;
    float warningTime = 0.0f;   // keeps the COLLISION banner from flickering

    DisableCursor();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_TAB)) { if (IsCursorHidden()) EnableCursor(); else DisableCursor(); }
        if (IsKeyPressed(KEY_R))
        {
            camera.position = (Vector3){ 0.0f, EYE_HEIGHT, 8.0f };
            yaw = -PI/2.0f; pitch = 0.0f;
        }

        // --- LOOK ------------------------------------------------------------
        if (IsCursorHidden())
        {
            Vector2 mouse = GetMouseDelta();
            yaw   += mouse.x*0.003f;
            pitch -= mouse.y*0.003f;
            if (pitch >  1.55f) pitch =  1.55f;
            if (pitch < -1.55f) pitch = -1.55f;
        }

        Vector3 forward = { cosf(pitch)*cosf(yaw), sinf(pitch), cosf(pitch)*sinf(yaw) };
        Vector3 right   = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0,1,0}));
        Vector3 flatForward = Vector3Normalize((Vector3){ forward.x, 0.0f, forward.z });

        // --- HOW FAR I WANT TO MOVE ------------------------------------------
        Vector3 dir = { 0 };
        if (IsKeyDown(KEY_W)) dir = Vector3Add(dir, flatForward);
        if (IsKeyDown(KEY_S)) dir = Vector3Subtract(dir, flatForward);
        if (IsKeyDown(KEY_D)) dir = Vector3Add(dir, right);
        if (IsKeyDown(KEY_A)) dir = Vector3Subtract(dir, right);
        if (Vector3Length(dir) > 0.0f) dir = Vector3Normalize(dir);

        float speed = IsKeyDown(KEY_LEFT_SHIFT) ? 8.0f : 4.0f;
        float dx = dir.x*speed*dt;
        float dz = dir.z*speed*dt;

        // --- MOVE AND CHECK, ONE AXIS AT A TIME ------------------------------
        for (int i = 0; i < MAX_BOXES; i++) boxes[i].touched = false;
        touchedNow = -1;

        // X axis
        Vector3 attempt = camera.position;
        attempt.x += dx;
        int hit = CollisionIndex(attempt, boxes, MAX_BOXES);
        if (hit >= 0) { boxes[hit].touched = true; touchedNow = hit; }   // do not advance on X
        else camera.position.x = attempt.x;

        // Z axis
        attempt = camera.position;
        attempt.z += dz;
        hit = CollisionIndex(attempt, boxes, MAX_BOXES);
        if (hit >= 0) { boxes[hit].touched = true; touchedNow = hit; }   // do not advance on Z
        else camera.position.z = attempt.z;

        if (touchedNow >= 0) warningTime = 0.35f;
        else if (warningTime > 0.0f) warningTime -= dt;

        camera.position.y = EYE_HEIGHT;                  // always standing
        camera.target = Vector3Add(camera.position, forward);

        // --- DRAW -------------------------------------------------------------
        BeginDrawing();
            ClearBackground((Color){ 16, 18, 24, 255 });

            BeginMode3D(camera);
                DrawPlane((Vector3){0,0,0}, (Vector2){24,24}, (Color){ 34, 38, 46, 255 });
                DrawGrid(24, 1.0f);

                for (int i = 0; i < MAX_BOXES; i++)
                {
                    Color c = boxes[i].touched ? RED : boxes[i].color;
                    DrawCubeV(boxes[i].position, boxes[i].size, Fade(c, 0.85f));
                    DrawCubeWiresV(boxes[i].position, boxes[i].size,
                                   boxes[i].touched ? WHITE : Fade(BLACK, 0.5f));
                }

                // Axes at the origin, as a compass
                DrawLine3D((Vector3){0,0.02f,0}, (Vector3){3,0.02f,0}, RED);
                DrawLine3D((Vector3){0,0,0},     (Vector3){0,3,0},     GREEN);
                DrawLine3D((Vector3){0,0.02f,0}, (Vector3){0,0.02f,3}, BLUE);
            EndMode3D();

            DrawCircleLines(GetScreenWidth()/2, GetScreenHeight()/2, 4, RAYWHITE);

            DrawRectangle(10, 10, 400, 118, Fade(BLACK, 0.65f));
            DrawText(TextFormat("X %6.2f   Y %6.2f   Z %6.2f",
                     camera.position.x, camera.position.y, camera.position.z), 20, 20, 18, RAYWHITE);
            DrawText(TextFormat("looking: yaw %.0f deg   pitch %.0f deg", yaw*RAD2DEG, pitch*RAD2DEG), 20, 46, 16, GRAY);
            DrawText("WASD move   mouse look   SHIFT run", 20, 74, 14, GRAY);
            DrawText("TAB release mouse   R reset   ESC quit", 20, 94, 14, GRAY);
            DrawText("walls stop you: movement is resolved per axis", 20, 112, 12, DARKGRAY);

            if (warningTime > 0.0f)
            {
                const char *txt = "YOU HIT SOMETHING";
                int width = MeasureText(txt, 34);
                DrawRectangle(GetScreenWidth()/2 - width/2 - 16, 24, width + 32, 52, Fade(RED, 0.3f));
                DrawText(txt, GetScreenWidth()/2 - width/2, 36, 34, RED);
            }

            DrawFPS(GetScreenWidth() - 90, 10);
        EndDrawing();
    }

    EnableCursor();
    CloseWindow();
    return 0;
}
