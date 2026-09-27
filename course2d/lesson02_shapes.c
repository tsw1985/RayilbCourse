/* =============================================================================
   LESSON 02 - Drawing shapes: coordinates, colors and order
   -----------------------------------------------------------------------------
   THE SCREEN GRID (2D). The origin (0,0) is the TOP-LEFT corner:

       (0,0) ---------------- X grows right ----> (800,0)
         |
         |
       Y grows DOWN
         |
         v
       (0,450)

   Careful: this is the opposite of school maths, where Y grows up. In 3D
   (course3d) Y goes back to growing up. Do not mix them.

   THE PAINTER'S ALGORITHM. Drawing is like painting on a canvas: whatever you
   draw LAST covers whatever you drew before. There is no z-order, no layers,
   no sorting. The order of your Draw* calls IS the order of the layers.

   A Color is four bytes: { red, green, blue, alpha }, each from 0 to 255.
   Alpha 255 = opaque, 0 = invisible. Fade(color, 0.5f) gives you a half
   transparent copy of a color without writing the struct out by hand.

   Controls: SPACE toggles the draw order, so you can see it matters.
   ============================================================================= */

#include "raylib.h"

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);   // MSAA = smooth edges
    InitWindow(900, 520, "Lesson 02 - Shapes, colors and order");
    SetTargetFPS(60);

    bool circleOnTop = true;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_SPACE)) circleOnTop = !circleOnTop;

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            // --- The coordinate grid, so you can see where things land -------
            for (int x = 0; x <= 900; x += 50) DrawLine(x, 0, x, 520, (Color){ 225, 225, 225, 255 });
            for (int y = 0; y <= 520; y += 50) DrawLine(0, y, 900, y, (Color){ 225, 225, 225, 255 });
            DrawLine(0, 0, 120, 0, RED);     DrawText("+X", 125, -2, 16, RED);
            DrawLine(0, 0, 0, 120, GREEN);   DrawText("+Y", 6, 120, 16, GREEN);
            DrawCircle(0, 0, 6, BLACK);      DrawText("(0,0)", 12, 8, 14, DARKGRAY);

            // --- Rectangles ---------------------------------------------------
            // DrawRectangle(x, y, width, height, color)
            // x,y is the TOP-LEFT corner, NOT the center. This trips up everyone.
            DrawRectangle(80, 100, 120, 80, MAROON);
            DrawText("rectangle", 80, 186, 14, DARKGRAY);

            // Same rectangle, outline only, and with a thickness
            DrawRectangleLinesEx((Rectangle){ 80, 220, 120, 80 }, 3, MAROON);
            DrawText("outline", 80, 306, 14, DARKGRAY);

            // A Rectangle is a struct { x, y, width, height } you will use a lot
            Rectangle box = { 80, 340, 120, 80 };
            DrawRectangleRounded(box, 0.3f, 8, ORANGE);
            DrawText("rounded", 80, 426, 14, DARKGRAY);

            // --- Circles ------------------------------------------------------
            // DrawCircle(centerX, centerY, radius, color)
            // Here x,y IS the center. Rectangles use a corner, circles use the
            // center: no, it is not consistent, just memorize it.
            DrawCircle(300, 140, 40, DARKBLUE);
            DrawText("circle", 260, 186, 14, DARKGRAY);

            DrawCircleLines(300, 260, 40, DARKBLUE);
            DrawText("outline", 260, 306, 14, DARKGRAY);

            // --- Lines and triangles ------------------------------------------
            DrawLine(420, 100, 540, 180, BLACK);
            DrawLineEx((Vector2){ 420, 200 }, (Vector2){ 540, 280 }, 5, BLACK);
            DrawText("line / thick line", 420, 300, 14, DARKGRAY);

            // Vertices must go counter-clockwise or the triangle is culled away
            DrawTriangle((Vector2){ 480, 340 }, (Vector2){ 420, 440 }, (Vector2){ 540, 440 }, DARKGREEN);
            DrawText("triangle", 420, 450, 14, DARKGRAY);

            // --- Transparency --------------------------------------------------
            DrawRectangle(620, 100, 120, 120, RED);
            DrawRectangle(680, 160, 120, 120, Fade(BLUE, 0.5f));   // 50% alpha
            DrawText("Fade(BLUE, 0.5f)", 620, 286, 14, DARKGRAY);

            // --- Draw order ----------------------------------------------------
            // Whatever is drawn last wins. Press SPACE and watch.
            if (circleOnTop)
            {
                DrawRectangle(640, 340, 100, 100, DARKPURPLE);
                DrawCircle(700, 400, 45, GOLD);            // painted last -> on top
            }
            else
            {
                DrawCircle(700, 400, 45, GOLD);
                DrawRectangle(640, 340, 100, 100, DARKPURPLE); // painted last -> on top
            }
            DrawText(circleOnTop ? "circle last" : "square last", 620, 450, 14, DARKGRAY);

            DrawText("SPACE: swap draw order", 620, 470, 14, GRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
