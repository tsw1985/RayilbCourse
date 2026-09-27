/* =============================================================================
   CHEATSHEET MODULE 2/10 - rcore: input (keyboard, mouse, gamepad, touch, drop)
   -----------------------------------------------------------------------------
   The cheatsheet lists input in five separate blocks. They all follow the same
   naming pattern, which is worth learning once:

     IsXPressed()   -> true for ONE frame, when it goes down
     IsXDown()      -> true EVERY frame while it is held
     IsXReleased()  -> true for ONE frame, when it comes up
     IsXUp()        -> true every frame while it is NOT held
     GetX()         -> a value rather than a yes/no

   Keyboard, mouse buttons and gamepad buttons all obey it.

   Controls: type anything, move and click the mouse, plug in a gamepad,
             drag a file onto the window. C clears the typed text.
   ============================================================================= */

#include "raylib.h"
#include <string.h>

#define MAX_TYPED 64

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(980, 640, "Module 2 - rcore: input");
    SetTargetFPS(60);

    char typed[MAX_TYPED + 1] = { 0 };
    int  typedLength = 0;

    char droppedInfo[256] = "drag a file onto this window";

    while (!WindowShouldClose())
    {
        // =====================================================================
        // KEYBOARD
        // =====================================================================
        // GetCharPressed returns UNICODE characters with the layout and shift
        // already applied: this is what you want for a name field.
        // GetKeyPressed returns raw key CODES: that is what you want for
        // "press any key to rebind".
        int c = GetCharPressed();
        while (c > 0)
        {
            if ((c >= 32) && (c <= 125) && (typedLength < MAX_TYPED))
            {
                typed[typedLength++] = (char)c;
                typed[typedLength] = '\0';
            }
            c = GetCharPressed();       // the queue can hold several per frame
        }

        if (IsKeyPressed(KEY_BACKSPACE) && typedLength > 0) typed[--typedLength] = '\0';
        if (IsKeyPressed(KEY_C)) { typed[0] = '\0'; typedLength = 0; }

        // =====================================================================
        // DROPPED FILES
        // =====================================================================
        if (IsFileDropped())
        {
            FilePathList dropped = LoadDroppedFiles();

            if (dropped.count > 0)
                TextCopy(droppedInfo, TextFormat("%i file(s), first: %s",
                         dropped.count, GetFileName(dropped.paths[0])));

            // ALWAYS unload it: the list is heap allocated by raylib
            UnloadDroppedFiles(dropped);
        }

        Vector2 mouse = GetMousePosition();

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            int y = 16;
            DrawText("rcore - INPUT", 20, y, 24, DARKGRAY); y += 38;

            // ---- keyboard -----------------------------------------------------
            DrawText("KEYBOARD", 20, y, 18, MAROON); y += 24;
            DrawText(TextFormat("IsKeyDown(SPACE)     %s", IsKeyDown(KEY_SPACE) ? "YES" : "no"), 30, y, 16, BLACK); y += 20;
            DrawText(TextFormat("IsKeyPressed(SPACE)  %s", IsKeyPressed(KEY_SPACE) ? "YES" : "no"), 30, y, 16, BLACK); y += 20;
            DrawText(TextFormat("IsKeyUp(SPACE)       %s", IsKeyUp(KEY_SPACE) ? "YES" : "no"), 30, y, 16, BLACK); y += 20;
            DrawText(TextFormat("GetKeyPressed()      %i (raw code)", GetKeyPressed()), 30, y, 16, BLACK); y += 20;
            DrawText(TextFormat("typed: \"%s\"", typed), 30, y, 16, DARKBLUE); y += 28;

            // ---- mouse --------------------------------------------------------
            DrawText("MOUSE", 20, y, 18, MAROON); y += 24;
            DrawText(TextFormat("GetMousePosition()  %.0f, %.0f", mouse.x, mouse.y), 30, y, 16, BLACK); y += 20;
            DrawText(TextFormat("GetMouseDelta()     %.1f, %.1f", GetMouseDelta().x, GetMouseDelta().y), 30, y, 16, BLACK); y += 20;
            DrawText(TextFormat("GetMouseWheelMove() %.1f", GetMouseWheelMove()), 30, y, 16, BLACK); y += 20;
            DrawText(TextFormat("LEFT %s   RIGHT %s   MIDDLE %s",
                     IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? "DOWN" : "up",
                     IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ? "DOWN" : "up",
                     IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) ? "DOWN" : "up"), 30, y, 16, BLACK); y += 28;

            // ---- gamepad ------------------------------------------------------
            DrawText("GAMEPAD", 20, y, 18, MAROON); y += 24;
            if (IsGamepadAvailable(0))
            {
                DrawText(TextFormat("0: \"%s\"  axes: %i", GetGamepadName(0), GetGamepadAxisCount(0)), 30, y, 16, BLACK); y += 20;
                DrawText(TextFormat("left stick  %.2f, %.2f",
                         GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X),
                         GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y)), 30, y, 16, BLACK); y += 20;
                DrawText(TextFormat("button down: %i", GetGamepadButtonPressed()), 30, y, 16, BLACK); y += 20;
            }
            else { DrawText("none connected", 30, y, 16, GRAY); y += 20; }
            y += 10;

            // ---- touch and gestures -------------------------------------------
            // On a desktop the mouse counts as one touch point, so this works
            // without a touchscreen.
            DrawText("TOUCH / GESTURES", 20, y, 18, MAROON); y += 24;
            DrawText(TextFormat("touch points: %i", GetTouchPointCount()), 30, y, 16, BLACK); y += 20;

            int gesture = GetGestureDetected();
            const char *gestureName = "NONE";
            switch (gesture)
            {
                case GESTURE_TAP:        gestureName = "TAP"; break;
                case GESTURE_DOUBLETAP:  gestureName = "DOUBLETAP"; break;
                case GESTURE_HOLD:       gestureName = "HOLD"; break;
                case GESTURE_DRAG:       gestureName = "DRAG"; break;
                case GESTURE_SWIPE_LEFT: gestureName = "SWIPE LEFT"; break;
                case GESTURE_SWIPE_RIGHT:gestureName = "SWIPE RIGHT"; break;
                case GESTURE_PINCH_IN:   gestureName = "PINCH IN"; break;
                case GESTURE_PINCH_OUT:  gestureName = "PINCH OUT"; break;
                default: break;
            }
            DrawText(TextFormat("GetGestureDetected(): %s", gestureName), 30, y, 16, DARKGREEN); y += 28;

            // ---- dropped files -------------------------------------------------
            DrawText("DROPPED FILES", 20, y, 18, MAROON); y += 24;
            DrawText(droppedInfo, 30, y, 16, DARKPURPLE);

            // A cursor drawn by us, following the mouse
            DrawCircleLines((int)mouse.x, (int)mouse.y, 14, Fade(BLUE, 0.7f));

            DrawRectangle(0, GetScreenHeight() - 28, GetScreenWidth(), 28, Fade(BLACK, 0.75f));
            DrawText("type anything   C clears   drag a file in   plug a gamepad",
                     10, GetScreenHeight() - 21, 15, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
