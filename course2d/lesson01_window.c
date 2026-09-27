/* =============================================================================
   LESSON 01 - The smallest raylib program, line by line
   -----------------------------------------------------------------------------
   Every raylib program has the exact same skeleton. Learn this shape once and
   you can read any raylib example on the internet.

       InitWindow(...)              <- 1. open the window, once
       while (!WindowShouldClose()) <- 2. the frame loop, ~60 times per second
       {
           // update your variables
           BeginDrawing();          <- 3. "I am going to paint now"
               ClearBackground(..); <-    wipe last frame's picture
               ... draw stuff ...   <-    paint this frame's picture
           EndDrawing();            <- 4. "done, show it on screen"
       }
       CloseWindow()                <- 5. close everything, once

   The key idea, and the one that surprises people coming from other toolkits:
   raylib is IMMEDIATE MODE. There are no objects that "stay" on screen. Nothing
   is remembered. Every single frame you repaint the whole picture from scratch.
   If you stop calling DrawCircle(), the circle is simply gone next frame.

   Controls: ESC quits (WindowShouldClose returns true on ESC or the X button).
   ============================================================================= */

#include "raylib.h"

int main(void)
{
    // -------------------------------------------------------------------------
    // 1. SETUP - runs once
    // -------------------------------------------------------------------------

    // Flags must be set BEFORE InitWindow, they configure how it is created.
    // FLAG_VSYNC_HINT syncs drawing with the monitor refresh: no tearing.
    SetConfigFlags(FLAG_VSYNC_HINT);

    // Creates the window AND the OpenGL context. Nothing raylib works before this.
    InitWindow(800, 450, "Lesson 01 - The frame loop");

    // Caps the loop at 60 iterations per second so we do not burn the CPU.
    SetTargetFPS(60);

    int frameCounter = 0;   // an ordinary variable, it survives between frames

    // -------------------------------------------------------------------------
    // 2. THE FRAME LOOP - runs ~60 times per second until you quit
    // -------------------------------------------------------------------------
    while (!WindowShouldClose())
    {
        // --- UPDATE: change your variables here, draw nothing --------------
        frameCounter++;

        // --- DRAW: paint the whole picture, change nothing -----------------
        BeginDrawing();

            // The window still holds the previous frame's pixels. If you skip
            // this line you get smears and garbage. Always clear first.
            ClearBackground(RAYWHITE);

            DrawText("This text is repainted every single frame", 60, 150, 20, DARKGRAY);
            DrawText(TextFormat("frame number: %i", frameCounter), 60, 190, 20, MAROON);
            DrawText(TextFormat("seconds since start: %.2f", GetTime()), 60, 220, 20, MAROON);
            DrawText(TextFormat("this frame took %.4f seconds", GetFrameTime()), 60, 250, 20, MAROON);

            DrawText("press ESC to quit", 60, 320, 16, GRAY);

            DrawFPS(700, 10);   // a helper that prints the current FPS

        // Two things happen here: the queued drawing is sent to the GPU, and
        // the finished image is swapped onto the screen (double buffering).
        // That is why nothing appears until EndDrawing is called.
        EndDrawing();
    }

    // -------------------------------------------------------------------------
    // 3. CLEANUP - runs once
    // -------------------------------------------------------------------------
    CloseWindow();

    return 0;
}
