/* =============================================================================
   CHEATSHEET MODULE 10/10 - raymath: vectors, matrices, quaternions
   -----------------------------------------------------------------------------
   raymath is a SEPARATE header with its own cheatsheet. It ships with raylib but
   is not included by raylib.h, so you have to ask for it:

       #include "raymath.h"

   It is header-only and every function is tiny and inlined, so using it costs
   nothing. Four families:

     Utils        Lerp, Clamp, Normalize, Remap, Wrap
     Vector2/3/4  the ones you will use every single day
     Matrix       4x4 transforms: translate, rotate, scale, project
     Quaternion   rotations that can be blended without breaking

   The three that matter most, and what they actually mean:

     Normalize    "keep the direction, throw away the length"
     DotProduct   how much two directions agree: 1 same, 0 perpendicular,
                  -1 opposite. This is how you test if something is in front
                  of you or behind you.
     CrossProduct (3D only) "give me the direction perpendicular to both"

   Controls: drag the two white dots. TAB switches the 2D/3D panel.
   ============================================================================= */

#include "raylib.h"
#include "raymath.h"

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1040, 700, "Module 10 - raymath");
    SetTargetFPS(60);

    Vector2 origin = { 300, 330 };
    Vector2 a = { 460, 240 };      // draggable
    Vector2 b = { 400, 440 };      // draggable
    int dragging = -1;

    Camera3D camera = { 0 };
    camera.position   = (Vector3){ 6.0f, 5.0f, 6.0f };
    camera.target     = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    bool show3D = false;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_TAB)) show3D = !show3D;

        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if (dragging == -1)
            {
                if (CheckCollisionPointCircle(mouse, a, 14)) dragging = 0;
                else if (CheckCollisionPointCircle(mouse, b, 14)) dragging = 1;
            }
            if (dragging == 0) a = mouse;
            if (dragging == 1) b = mouse;
        }
        else dragging = -1;

        if (show3D) UpdateCamera(&camera, CAMERA_ORBITAL);

        // ---- The actual maths ------------------------------------------------
        Vector2 va = Vector2Subtract(a, origin);       // vector from origin to a
        Vector2 vb = Vector2Subtract(b, origin);

        Vector2 sum  = Vector2Add(va, vb);
        Vector2 diff = Vector2Subtract(va, vb);
        float   dot  = Vector2DotProduct(Vector2Normalize(va), Vector2Normalize(vb));
        float   angle = Vector2Angle(va, vb)*RAD2DEG;
        float   lenA = Vector2Length(va);
        float   dist = Vector2Distance(a, b);
        Vector2 mid  = Vector2Lerp(a, b, 0.5f);
        Vector2 perp = Vector2Rotate(Vector2Normalize(va), PI/2.0f);

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            DrawText("raymath", 20, 14, 24, DARKGRAY);
            DrawText(TextFormat("TAB: showing the %s panel", show3D ? "3D (Vector3 + Quaternion)" : "2D (Vector2)"),
                     20, 44, 16, GRAY);

            if (!show3D)
            {
                // =============================================================
                // 2D PANEL
                // =============================================================
                DrawCircleV(origin, 5, BLACK);
                DrawText("origin", (int)origin.x - 24, (int)origin.y + 12, 14, GRAY);

                // The two draggable vectors
                DrawLineEx(origin, a, 3, RED);
                DrawCircleV(a, 10, RAYWHITE); DrawCircleLines((int)a.x, (int)a.y, 10, RED);
                DrawText("a", (int)a.x + 14, (int)a.y - 8, 20, RED);

                DrawLineEx(origin, b, 3, DARKBLUE);
                DrawCircleV(b, 10, RAYWHITE); DrawCircleLines((int)b.x, (int)b.y, 10, DARKBLUE);
                DrawText("b", (int)b.x + 14, (int)b.y - 8, 20, DARKBLUE);

                // Vector2Add: the parallelogram rule
                DrawLineEx(origin, Vector2Add(origin, sum), 2, Fade(DARKGREEN, 0.9f));
                DrawText("a + b", (int)(origin.x + sum.x) + 8, (int)(origin.y + sum.y), 16, DARKGREEN);
                DrawLineEx(a, Vector2Add(origin, sum), 1, Fade(DARKGREEN, 0.35f));
                DrawLineEx(b, Vector2Add(origin, sum), 1, Fade(DARKGREEN, 0.35f));

                // Vector2Subtract: from b to a
                DrawLineEx(b, a, 2, Fade(PURPLE, 0.8f));
                DrawText("a - b", (int)mid.x + 10, (int)mid.y - 24, 16, PURPLE);

                // Vector2Lerp: halfway between them
                DrawCircleV(mid, 6, ORANGE);
                DrawText("Lerp(a,b,0.5)", (int)mid.x + 10, (int)mid.y + 6, 14, ORANGE);

                // Vector2Rotate: a perpendicular unit vector, 90 degrees from a
                DrawLineEx(origin, Vector2Add(origin, Vector2Scale(perp, 70)), 2, GOLD);
                DrawText("Rotate(normalize(a), 90deg)",
                         (int)(origin.x + perp.x*70) + 8, (int)(origin.y + perp.y*70), 14, GOLD);

                // ---- the numbers ------------------------------------------
                int x = 660, y = 90;
                DrawRectangle(x - 16, y - 16, 380, 330, Fade(BLACK, 0.05f));
                DrawText("Vector2", x, y, 20, MAROON); y += 32;
                DrawText(TextFormat("a           (%.0f, %.0f)", va.x, va.y), x, y, 16, BLACK); y += 22;
                DrawText(TextFormat("b           (%.0f, %.0f)", vb.x, vb.y), x, y, 16, BLACK); y += 22;
                DrawText(TextFormat("Add         (%.0f, %.0f)", sum.x, sum.y), x, y, 16, DARKGREEN); y += 22;
                DrawText(TextFormat("Subtract    (%.0f, %.0f)", diff.x, diff.y), x, y, 16, PURPLE); y += 22;
                DrawText(TextFormat("Length(a)   %.1f", lenA), x, y, 16, BLACK); y += 22;
                DrawText(TextFormat("Distance    %.1f", dist), x, y, 16, BLACK); y += 22;
                DrawText(TextFormat("Normalize(a)(%.2f, %.2f)",
                         Vector2Normalize(va).x, Vector2Normalize(va).y), x, y, 16, BLACK); y += 22;
                DrawText(TextFormat("DotProduct  %+.2f", dot), x, y, 16, DARKBLUE); y += 20;
                DrawText(dot > 0.7f ? "  -> pointing the same way"
                       : dot < -0.7f ? "  -> pointing opposite ways"
                       : "  -> roughly perpendicular", x, y, 14, DARKBLUE); y += 26;
                DrawText(TextFormat("Angle       %.1f deg", angle), x, y, 16, BLACK); y += 30;

                DrawText("Utils", x, y, 20, MAROON); y += 28;
                DrawText(TextFormat("Lerp(0, 100, 0.25)  %.0f", Lerp(0, 100, 0.25f)), x, y, 15, BLACK); y += 20;
                DrawText(TextFormat("Clamp(150, 0, 100)  %.0f", Clamp(150, 0, 100)), x, y, 15, BLACK); y += 20;
                DrawText(TextFormat("Remap(5, 0,10, 0,1) %.2f", Remap(5, 0, 10, 0, 1)), x, y, 15, BLACK); y += 20;
                DrawText(TextFormat("Wrap(370, 0, 360)   %.0f", Wrap(370, 0, 360)), x, y, 15, BLACK);

                DrawText("drag the two white dots", 20, GetScreenHeight() - 34, 16, GRAY);
            }
            else
            {
                // =============================================================
                // 3D PANEL
                // =============================================================
                Vector3 v1 = { 3.0f, 0.0f, 0.0f };
                Vector3 v2 = { 0.0f, 0.0f, 3.0f };
                Vector3 cross = Vector3CrossProduct(v1, v2);     // -> straight up

                // A quaternion rotation, applied to a point
                Quaternion q = QuaternionFromAxisAngle((Vector3){ 0, 1, 0 }, (float)GetTime());
                Vector3 spun = Vector3RotateByQuaternion((Vector3){ 2.5f, 1.5f, 0.0f }, q);

                BeginMode3D(camera);
                    DrawGrid(10, 1.0f);

                    DrawLine3D(Vector3Zero(), v1, RED);
                    DrawLine3D(Vector3Zero(), v2, BLUE);
                    DrawLine3D(Vector3Zero(), cross, GREEN);

                    DrawSphere(v1, 0.12f, RED);
                    DrawSphere(v2, 0.12f, BLUE);
                    DrawSphere(cross, 0.16f, GREEN);

                    DrawSphere(spun, 0.2f, GOLD);
                    DrawLine3D(Vector3Zero(), spun, Fade(GOLD, 0.6f));

                    // Vector3Lerp between the two axes
                    for (float t = 0.0f; t <= 1.0f; t += 0.1f)
                        DrawSphere(Vector3Lerp(v1, v2, t), 0.06f, Fade(PURPLE, 0.8f));
                EndMode3D();

                int x = 660, y = 90;
                DrawRectangle(x - 16, y - 16, 380, 300, Fade(BLACK, 0.05f));
                DrawText("Vector3", x, y, 20, MAROON); y += 32;
                DrawText(TextFormat("v1            (%.0f, %.0f, %.0f)", v1.x, v1.y, v1.z), x, y, 16, RED); y += 22;
                DrawText(TextFormat("v2            (%.0f, %.0f, %.0f)", v2.x, v2.y, v2.z), x, y, 16, BLUE); y += 22;
                DrawText(TextFormat("CrossProduct  (%.0f, %.0f, %.0f)", cross.x, cross.y, cross.z), x, y, 16, DARKGREEN); y += 20;
                DrawText("  -> perpendicular to BOTH", x, y, 14, DARKGREEN); y += 26;
                DrawText(TextFormat("DotProduct    %.1f", Vector3DotProduct(v1, v2)), x, y, 16, BLACK); y += 20;
                DrawText("  -> 0 means perpendicular", x, y, 14, GRAY); y += 26;
                DrawText("Lerp: the purple dots", x, y, 16, PURPLE); y += 32;

                DrawText("Quaternion", x, y, 20, MAROON); y += 30;
                DrawText(TextFormat("FromAxisAngle(Y, %.1f rad)", (float)GetTime()), x, y, 15, BLACK); y += 20;
                DrawText(TextFormat("q = (%.2f, %.2f, %.2f, %.2f)", q.x, q.y, q.z, q.w), x, y, 15, BLACK); y += 20;
                DrawText(TextFormat("rotated point (%.2f, %.2f, %.2f)", spun.x, spun.y, spun.z), x, y, 15, GOLD); y += 26;
                DrawText("quaternions blend without", x, y, 14, GRAY); y += 18;
                DrawText("gimbal lock: that is why", x, y, 14, GRAY); y += 18;
                DrawText("animation systems use them", x, y, 14, GRAY);
            }

            DrawRectangle(0, GetScreenHeight() - 26, GetScreenWidth(), 26, Fade(BLACK, 0.8f));
            DrawText("TAB switch 2D / 3D panel   (2D: drag the white dots)",
                     10, GetScreenHeight() - 20, 15, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
