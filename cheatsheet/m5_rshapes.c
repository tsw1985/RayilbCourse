/* =============================================================================
   CHEATSHEET MODULE 5/10 - rshapes: 2D shapes, splines and collisions
   -----------------------------------------------------------------------------
   The whole rshapes module in one screen. It has three parts:

     DRAWING      pixels, lines, rectangles, circles, ellipses, rings, polygons
     SPLINES      smooth curves through a list of points
     COLLISIONS   rectangle, circle, point and line intersection tests

   The naming pattern that covers the whole module, and most of raylib:

       DrawCircle      loose int coordinates, filled
       DrawCircleV     same, but takes a Vector2
       DrawCircleLines outline instead of filled
       DrawCircleEx    extended: an extra parameter such as thickness
       DrawCirclePro   every knob: origin, rotation, scale

   Controls: drag the blue box with the mouse to test the collisions,
             TAB switches the spline type, SPACE shows the spline points.
   ============================================================================= */

#include "raylib.h"

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1080, 700, "Module 5 - rshapes");
    SetTargetFPS(60);

    Rectangle player = { 120, 470, 60, 60 };     // dragged by the mouse
    Rectangle target = { 640, 470, 120, 90 };
    Vector2   circle = { 900, 515 };
    float     circleRadius = 50;

    // Control points for the splines
    Vector2 points[5] = {
        { 120, 250 }, { 300, 150 }, { 480, 300 }, { 660, 160 }, { 840, 260 }
    };

    int splineType = 0;
    const char *splineNames[] = { "DrawSplineLinear", "DrawSplineBasis",
                                  "DrawSplineCatmullRom", "DrawSplineBezierQuadratic" };
    bool showPoints = true;
    int dragging = -1;

    while (!WindowShouldClose())
    {
        Vector2 mouse = GetMousePosition();

        if (IsKeyPressed(KEY_TAB)) splineType = (splineType + 1) % 4;
        if (IsKeyPressed(KEY_SPACE)) showPoints = !showPoints;

        // Drag the player box, or any spline control point
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if (dragging == -1)
            {
                if (CheckCollisionPointRec(mouse, player)) dragging = 100;
                else for (int i = 0; i < 5; i++)
                    if (CheckCollisionPointCircle(mouse, points[i], 12)) { dragging = i; break; }
            }

            if (dragging == 100) { player.x = mouse.x - player.width/2; player.y = mouse.y - player.height/2; }
            else if (dragging >= 0) points[dragging] = mouse;
        }
        else dragging = -1;

        // ---- COLLISION TESTS ------------------------------------------------
        bool hitRect   = CheckCollisionRecs(player, target);
        bool hitCircle = CheckCollisionCircleRec(circle, circleRadius, player);

        // GetCollisionRec gives you the OVERLAPPING area of two rectangles,
        // which is how you work out how deep the penetration is.
        Rectangle overlap = GetCollisionRec(player, target);

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            DrawText("rshapes", 20, 14, 24, DARKGRAY);

            // =================================================================
            // 1. DRAWING
            // =================================================================
            DrawText("DRAWING", 20, 52, 18, MAROON);

            int x = 30, y = 80;
            DrawPixel(x + 10, y + 20, BLACK);                                   DrawText("Pixel", x, y + 48, 13, GRAY);
            x += 70;
            DrawLine(x, y, x + 44, y + 40, BLACK);                              DrawText("Line", x, y + 48, 13, GRAY);
            x += 70;
            DrawLineEx((Vector2){x, y}, (Vector2){x + 44, y + 40}, 5, BLACK);    DrawText("LineEx", x, y + 48, 13, GRAY);
            x += 76;
            DrawLineBezier((Vector2){x, y + 40}, (Vector2){x + 44, y}, 3, DARKBLUE); DrawText("Bezier", x, y + 48, 13, GRAY);
            x += 76;
            DrawLineDashed((Vector2){x, y + 20}, (Vector2){x + 50, y + 20}, 6, 4, BLACK); DrawText("Dashed", x, y + 48, 13, GRAY);
            x += 80;
            DrawRectangle(x, y, 44, 40, MAROON);                                DrawText("Rectangle", x, y + 48, 13, GRAY);
            x += 86;
            DrawRectangleLinesEx((Rectangle){x, y, 44, 40}, 3, MAROON);         DrawText("LinesEx", x, y + 48, 13, GRAY);
            x += 80;
            DrawRectangleRounded((Rectangle){x, y, 44, 40}, 0.4f, 8, ORANGE);   DrawText("Rounded", x, y + 48, 13, GRAY);
            x += 80;
            DrawRectangleGradientV(x, y, 44, 40, RED, YELLOW);                  DrawText("GradientV", x, y + 48, 13, GRAY);
            x += 86;
            DrawCircle(x + 22, y + 20, 22, DARKBLUE);                           DrawText("Circle", x, y + 48, 13, GRAY);
            x += 70;
            DrawCircleGradient((Vector2){ x + 22, y + 20 }, 22, SKYBLUE, DARKBLUE);          DrawText("Gradient", x, y + 48, 13, GRAY);
            x += 76;
            DrawEllipse(x + 26, y + 20, 26, 16, DARKGREEN);                     DrawText("Ellipse", x, y + 48, 13, GRAY);

            x = 30; y = 160;
            DrawCircleSector((Vector2){x + 22, y + 22}, 22, 0, 240, 16, PURPLE); DrawText("Sector", x, y + 50, 13, GRAY);
            x += 76;
            DrawRing((Vector2){x + 24, y + 22}, 12, 24, 0, 300, 24, VIOLET);     DrawText("Ring", x, y + 50, 13, GRAY);
            x += 76;
            DrawPoly((Vector2){x + 24, y + 22}, 6, 24, 0, GOLD);                DrawText("Poly(6)", x, y + 50, 13, GRAY);
            x += 76;
            DrawPolyLinesEx((Vector2){x + 24, y + 22}, 5, 24, 0, 3, GOLD);      DrawText("PolyLinesEx", x, y + 50, 13, GRAY);
            x += 96;
            DrawTriangle((Vector2){x + 24, y}, (Vector2){x, y + 44}, (Vector2){x + 48, y + 44}, DARKGREEN);
            DrawText("Triangle (CCW!)", x - 6, y + 50, 13, GRAY);

            // =================================================================
            // 2. SPLINES - smooth curves through control points
            // =================================================================
            DrawText(TextFormat("SPLINES - %s   (TAB to change)", splineNames[splineType]), 20, 226, 18, MAROON);

            switch (splineType)
            {
                case 0: DrawSplineLinear(points, 5, 4, DARKBLUE); break;
                case 1: DrawSplineBasis(points, 5, 4, DARKBLUE); break;
                case 2: DrawSplineCatmullRom(points, 5, 4, DARKBLUE); break;
                // The Bezier variants read the array as control-point groups
                case 3: DrawSplineBezierQuadratic(points, 5, 4, DARKBLUE); break;
                default: break;
            }

            if (showPoints)
                for (int i = 0; i < 5; i++)
                {
                    DrawCircleV(points[i], 7, Fade(RED, 0.8f));
                    DrawText(TextFormat("%i", i), (int)points[i].x + 10, (int)points[i].y - 8, 14, RED);
                }

            // =================================================================
            // 3. COLLISIONS
            // =================================================================
            DrawText("COLLISIONS - drag the blue box", 20, 420, 18, MAROON);

            DrawRectangleRec(target, hitRect ? RED : DARKGRAY);
            DrawText("CheckCollisionRecs", (int)target.x - 20, (int)target.y + 96, 14, DARKGRAY);

            DrawCircleV(circle, circleRadius, hitCircle ? RED : DARKGREEN);
            DrawText("CheckCollisionCircleRec", (int)circle.x - 76, (int)circle.y + 60, 14, DARKGRAY);

            DrawRectangleRec(player, Fade(BLUE, 0.85f));
            DrawRectangleLinesEx(player, 2, BLACK);

            // The overlapping region of the two rectangles
            if (hitRect)
            {
                DrawRectangleRec(overlap, Fade(YELLOW, 0.9f));
                DrawText(TextFormat("GetCollisionRec  %.0f x %.0f", overlap.width, overlap.height),
                         (int)overlap.x, (int)overlap.y - 20, 15, BLACK);
            }

            // Point in shape: is the mouse over anything?
            DrawText(TextFormat("CheckCollisionPointRec(mouse, target)    %s",
                     CheckCollisionPointRec(mouse, target) ? "YES" : "no"), 20, 620, 16, DARKBLUE);
            DrawText(TextFormat("CheckCollisionPointCircle(mouse, circle) %s",
                     CheckCollisionPointCircle(mouse, circle, circleRadius) ? "YES" : "no"), 20, 642, 16, DARKBLUE);

            DrawRectangle(0, GetScreenHeight() - 26, GetScreenWidth(), 26, Fade(BLACK, 0.8f));
            DrawText("drag the blue box or the red spline points   TAB spline type   SPACE show points",
                     10, GetScreenHeight() - 20, 15, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
