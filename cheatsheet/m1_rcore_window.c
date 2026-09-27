/* =============================================================================
   CHEATSHEET MODULE 1/10 - rcore: window, monitor, cursor, timing
   -----------------------------------------------------------------------------
   The cheatsheet's rcore module is by far the biggest one. It is split here over
   four files; this is the part that deals with the window itself.

   The three groups of window functions, and when each one runs:

     SetConfigFlags(...)     BEFORE InitWindow. Decides how the window is BORN.
     SetWindowX(...)         AFTER InitWindow. Changes a living window.
     IsWindowX(...)          AFTER InitWindow. Asks it a question.

   Getting that order wrong is the single most common rcore mistake: a flag set
   after InitWindow is simply ignored, silently.

   Controls: F fullscreen, B borderless, R resizable, M maximize, N minimize,
             T toggle topmost, C cycle the cursor state, P take a screenshot.
   ============================================================================= */

#include "raylib.h"

int main(void)
{
    // ---- BEFORE the window exists -------------------------------------------
    // Flags are a bitmask; OR them together. A flag set after InitWindow does
    // nothing at all, which is why these two lines are in this order.
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

    InitWindow(960, 600, "Module 1 - rcore: window");

    // ---- AFTER the window exists --------------------------------------------
    SetWindowMinSize(480, 320);     // only meaningful with FLAG_WINDOW_RESIZABLE
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);           // stop ESC from closing: we handle quitting

    int cursorState = 0;            // 0 normal, 1 hidden, 2 disabled (locked)
    char lastShot[128] = "none yet";

    while (!WindowShouldClose())
    {
        // ---- WINDOW STATE CHANGES -------------------------------------------
        if (IsKeyPressed(KEY_F)) ToggleFullscreen();          // real fullscreen mode
        if (IsKeyPressed(KEY_B)) ToggleBorderlessWindowed();  // fake fullscreen, faster to switch
        if (IsKeyPressed(KEY_M)) { if (IsWindowMaximized()) RestoreWindow(); else MaximizeWindow(); }
        if (IsKeyPressed(KEY_N)) MinimizeWindow();
        if (IsKeyPressed(KEY_T)) SetWindowState(IsWindowState(FLAG_WINDOW_TOPMOST)
                                                ? 0 : FLAG_WINDOW_TOPMOST);

        // ---- CURSOR ----------------------------------------------------------
        if (IsKeyPressed(KEY_C))
        {
            cursorState = (cursorState + 1) % 3;
            if (cursorState == 0) { ShowCursor(); EnableCursor(); }
            else if (cursorState == 1) HideCursor();      // invisible, still free
            else DisableCursor();                        // invisible AND locked
        }

        // ---- SCREENSHOT ------------------------------------------------------
        // Saved next to the working directory, not next to the executable.
        if (IsKeyPressed(KEY_P))
        {
            TakeScreenshot("screenshot.png");
            TextCopy(lastShot, TextFormat("screenshot.png in %s", GetWorkingDirectory()));
        }

        if (IsKeyPressed(KEY_ESCAPE)) break;   // our own quit, since SetExitKey(KEY_NULL)

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            int y = 20;
            DrawText("rcore - WINDOW", 20, y, 24, DARKGRAY); y += 40;

            // ---- ASKING THE WINDOW QUESTIONS ---------------------------------
            DrawText(TextFormat("window size      %i x %i", GetScreenWidth(), GetScreenHeight()), 20, y, 18, BLACK); y += 24;
            DrawText(TextFormat("render size      %i x %i  (differs on HighDPI)",
                     GetRenderWidth(), GetRenderHeight()), 20, y, 18, BLACK); y += 24;
            DrawText(TextFormat("window position  %.0f, %.0f",
                     GetWindowPosition().x, GetWindowPosition().y), 20, y, 18, BLACK); y += 24;
            DrawText(TextFormat("dpi scale        %.2f, %.2f",
                     GetWindowScaleDPI().x, GetWindowScaleDPI().y), 20, y, 18, BLACK); y += 34;

            DrawText(TextFormat("fullscreen   %s", IsWindowFullscreen() ? "yes" : "no"), 20, y, 18, DARKBLUE); y += 24;
            DrawText(TextFormat("maximized    %s", IsWindowMaximized() ? "yes" : "no"), 20, y, 18, DARKBLUE); y += 24;
            DrawText(TextFormat("focused      %s", IsWindowFocused() ? "yes" : "no"), 20, y, 18, DARKBLUE); y += 24;
            DrawText(TextFormat("resized this frame  %s", IsWindowResized() ? "YES" : "no"), 20, y, 18, DARKBLUE); y += 34;

            // ---- MONITORS -----------------------------------------------------
            int monitor = GetCurrentMonitor();
            DrawText(TextFormat("monitor %i of %i: \"%s\"", monitor, GetMonitorCount(),
                     GetMonitorName(monitor)), 20, y, 18, MAROON); y += 24;
            DrawText(TextFormat("  %i x %i at %i Hz", GetMonitorWidth(monitor),
                     GetMonitorHeight(monitor), GetMonitorRefreshRate(monitor)), 20, y, 18, MAROON); y += 34;

            // ---- TIMING -------------------------------------------------------
            DrawText(TextFormat("GetFPS()        %i", GetFPS()), 20, y, 18, DARKGREEN); y += 24;
            DrawText(TextFormat("GetFrameTime()  %.4f s", GetFrameTime()), 20, y, 18, DARKGREEN); y += 24;
            DrawText(TextFormat("GetTime()       %.2f s", GetTime()), 20, y, 18, DARKGREEN); y += 34;

            DrawText(TextFormat("cursor: %s",
                     cursorState == 0 ? "normal" : (cursorState == 1 ? "hidden" : "disabled (locked)")),
                     20, y, 18, DARKPURPLE); y += 24;
            DrawText(TextFormat("screenshot: %s", lastShot), 20, y, 14, GRAY);

            DrawRectangle(0, GetScreenHeight() - 30, GetScreenWidth(), 30, Fade(BLACK, 0.75f));
            DrawText("F fullscreen  B borderless  M maximize  N minimize  T topmost  C cursor  P shot  ESC quit",
                     10, GetScreenHeight() - 22, 15, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
