/* =============================================================================
   CHEATSHEET MODULE 7/10 - rtext: fonts, measuring, and text utilities
   -----------------------------------------------------------------------------
   Two halves that people do not expect to find together:

     1. DRAWING TEXT   fonts, sizes, spacing, rotation, measuring
     2. TEXT UTILITIES a small string library (TextFormat, TextSplit,
                       TextReplace, TextToUpper...) that exists because C's
                       own string handling is painful and raylib did not want
                       to force you into <string.h> for everything.

   The font story:

     GetFontDefault()   the built-in 10px bitmap font. Free, always there.
     LoadFont(file)     a .ttf/.otf/.fnt at its default size (32)
     LoadFontEx(...)    the one you actually want: choose the size and the
                        character set. Load it at the size you will draw it.

   A font is loaded at a FIXED pixel size. Draw it much bigger than that and it
   goes blurry; that is not a bug, it is a bitmap being stretched.

   Controls: UP/DOWN size, LEFT/RIGHT spacing, R rotate, TAB switch the demo text.
   ============================================================================= */

#include "raylib.h"

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1040, 700, "Module 7 - rtext");
    SetTargetFPS(60);

    // The default font is always available and needs no loading or unloading.
    Font defaultFont = GetFontDefault();

    float size = 32.0f;
    float spacing = 2.0f;
    float rotation = 0.0f;

    const char *samples[] = {
        "The quick brown fox",
        "raylib is simple and easy-to-use",
        "1234567890 !@#$%^&*()"
    };
    int sample = 0;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (IsKeyDown(KEY_UP))    size += 30.0f*dt;
        if (IsKeyDown(KEY_DOWN))  size -= 30.0f*dt;
        if (size < 8.0f)  size = 8.0f;
        if (size > 90.0f) size = 90.0f;

        if (IsKeyDown(KEY_RIGHT)) spacing += 6.0f*dt;
        if (IsKeyDown(KEY_LEFT))  spacing -= 6.0f*dt;
        if (spacing < 0.0f) spacing = 0.0f;

        if (IsKeyDown(KEY_R)) rotation += 40.0f*dt;
        if (IsKeyPressed(KEY_TAB)) sample = (sample + 1) % 3;

        const char *text = samples[sample];

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            DrawText("rtext", 20, 14, 24, DARKGRAY);

            int y = 56;

            // =================================================================
            // 1. DRAWING
            // =================================================================
            DrawText("DRAWING", 20, y, 18, MAROON); y += 28;

            // The simple one: default font, integer position, no spacing control
            DrawText(text, 30, y, (int)size, BLACK);
            DrawText("DrawText - default font", 30, y + (int)size + 4, 13, GRAY);
            y += (int)size + 30;

            // The full one: any font, float position, explicit spacing
            DrawTextEx(defaultFont, text, (Vector2){ 30, (float)y }, size, spacing, DARKBLUE);
            DrawText("DrawTextEx - font, size, spacing", 30, y + (int)size + 4, 13, GRAY);
            y += (int)size + 30;

            // With an origin and a rotation
            DrawTextPro(defaultFont, "rotated", (Vector2){ 140, (float)y + 30 },
                        (Vector2){ 0, 0 }, rotation, size, spacing, MAROON);
            DrawText("DrawTextPro - origin + rotation (hold R)", 260, y + 20, 13, GRAY);
            y += 80;

            // =================================================================
            // 2. MEASURING - the only way to centre or wrap text
            // =================================================================
            DrawText("MEASURING", 20, y, 18, MAROON); y += 28;

            int widthSimple = MeasureText(text, (int)size);
            Vector2 widthEx = MeasureTextEx(defaultFont, text, size, spacing);

            DrawText(TextFormat("MeasureText()    %i px wide", widthSimple), 30, y, 16, BLACK); y += 22;
            DrawText(TextFormat("MeasureTextEx()  %.1f x %.1f px", widthEx.x, widthEx.y), 30, y, 16, BLACK); y += 26;

            // Centring: measure, then subtract half
            {
                const char *centred = "centred with MeasureText";
                int w = MeasureText(centred, 22);
                DrawRectangle(GetScreenWidth()/2 - w/2 - 8, y - 4, w + 16, 30, Fade(GOLD, 0.35f));
                DrawText(centred, GetScreenWidth()/2 - w/2, y, 22, DARKGRAY);
            }
            y += 42;

            // =================================================================
            // 3. TEXT UTILITIES - raylib's little string library
            // =================================================================
            DrawText("TEXT UTILITIES", 20, y, 18, MAROON); y += 28;

            const char *src = "level_02_boss";

            DrawText(TextFormat("TextFormat(\"%%i-%%s\", 7, \"hp\")   %s", TextFormat("%i-%s", 7, "hp")), 30, y, 15, BLACK); y += 20;
            DrawText(TextFormat("TextToUpper(\"%s\")   %s", src, TextToUpper(src)), 30, y, 15, BLACK); y += 20;
            DrawText(TextFormat("TextReplace(\"_\", \" \")   %s", TextReplace(src, "_", " ")), 30, y, 15, BLACK); y += 20;
            DrawText(TextFormat("TextSubtext(src, 6, 2)   \"%s\"", TextSubtext(src, 6, 2)), 30, y, 15, BLACK); y += 20;
            DrawText(TextFormat("TextFindIndex(src, \"boss\")   %i", TextFindIndex(src, "boss")), 30, y, 15, BLACK); y += 20;
            DrawText(TextFormat("TextLength(src)   %i", TextLength(src)), 30, y, 15, BLACK); y += 20;

            // TextSplit returns pointers into one internal buffer
            {
                int count = 0;
                char **parts = TextSplit(src, '_', &count);
                DrawText(TextFormat("TextSplit(src, '_')   %i parts:", count), 30, y, 15, DARKGREEN);
                for (int i = 0; i < count; i++)
                    DrawText(TextFormat("[%s]", parts[i]), 320 + i*90, y, 15, DARKGREEN);
                y += 26;
            }

            // =================================================================
            // 4. UNICODE - one character is not one byte
            // =================================================================
            DrawText("CODEPOINTS", 20, y, 18, MAROON); y += 26;
            {
                const char *utf8 = "anos \xc3\xb1 \xe2\x86\x92";   // "anos n-tilde arrow", as raw UTF-8
                int codepointCount = 0;
                int *codepoints = LoadCodepoints(utf8, &codepointCount);

                DrawText(TextFormat("bytes: %i    codepoints: %i   <- these differ in UTF-8",
                         TextLength(utf8), codepointCount), 30, y, 15, DARKPURPLE); y += 20;
                DrawText(TextFormat("first three: U+%04X U+%04X U+%04X",
                         codepoints[0], codepoints[1], codepoints[2]), 30, y, 15, DARKPURPLE);

                UnloadCodepoints(codepoints);
            }

            DrawRectangle(0, GetScreenHeight() - 26, GetScreenWidth(), 26, Fade(BLACK, 0.8f));
            DrawText(TextFormat("UP/DOWN size %.0f   LEFT/RIGHT spacing %.1f   R rotate   TAB text", size, spacing),
                     10, GetScreenHeight() - 20, 15, RAYWHITE);
        EndDrawing();
    }

    // The DEFAULT font must NOT be unloaded: raylib owns it.
    // A font from LoadFont/LoadFontEx must be, with UnloadFont().
    CloseWindow();
    return 0;
}
