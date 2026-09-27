/* =============================================================================
   CHEATSHEET MODULE 4/10 - rcore: files, random, logging, clipboard, data
   -----------------------------------------------------------------------------
   The quiet corner of rcore. None of it draws anything, all of it is the plumbing
   you need the day you add a save file, a settings menu or a level loader.

   Four groups:

     FILE SYSTEM      paths, extensions, listing directories. Pure string work,
                      it never touches the disk except for the Exists checks.
     FILE DATA        actually reading and writing bytes or text.
     RANDOM           GetRandomValue, plus a shuffled sequence with no repeats.
     MISC             TraceLog, the clipboard, compression, base64.

   This example WRITES A FILE (savegame.txt) in the working directory when you
   press S, and reads it back with L.

   Controls: R new random numbers, S save, L load, D delete the save,
             C copy to clipboard, Z compress a test buffer.
   ============================================================================= */

#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>      // remove()
#include <string.h>

#define SAVE_FILE "savegame.txt"

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(980, 660, "Module 4 - rcore: files, random, misc");
    SetTargetFPS(60);

    // TraceLog is raylib's printf-to-the-terminal. The level decides how much
    // noise you get; LOG_WARNING is a good default for a released game.
    SetTraceLogLevel(LOG_INFO);
    TraceLog(LOG_INFO, "module 4 started");

    // ---- RANDOM --------------------------------------------------------------
    // SetRandomSeed makes a run repeatable, which is what you want for testing
    // and for "daily challenge" style levels.
    SetRandomSeed(1234);
    int dice[6];
    for (int i = 0; i < 6; i++) dice[i] = GetRandomValue(1, 6);

    // A shuffled sequence with NO repeats: card decks, level orders, spawn slots.
    int *deck = LoadRandomSequence(10, 1, 10);

    char status[256] = "nothing done yet";
    char loaded[256] = "(not loaded)";
    int  score = 4200;
    int  compressedSize = 0, rawSize = 0;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_R))
        {
            for (int i = 0; i < 6; i++) dice[i] = GetRandomValue(1, 6);
            UnloadRandomSequence(deck);
            deck = LoadRandomSequence(10, 1, 10);
            TextCopy(status, "new random values");
        }

        // ---- WRITING A FILE --------------------------------------------------
        if (IsKeyPressed(KEY_S))
        {
            const char *text = TextFormat("score=%i\nlevel=%i\nname=%s\n", score, 3, "player one");

            if (SaveFileText(SAVE_FILE, (char *)text))
                TextCopy(status, TextFormat("saved to %s/%s", GetWorkingDirectory(), SAVE_FILE));
            else
                TextCopy(status, "save FAILED");
        }

        // ---- READING A FILE --------------------------------------------------
        if (IsKeyPressed(KEY_L))
        {
            if (FileExists(SAVE_FILE))
            {
                char *text = LoadFileText(SAVE_FILE);       // heap allocated
                TextCopy(loaded, text);
                UnloadFileText(text);                       // you must free it
                TextCopy(status, TextFormat("loaded %i bytes", GetFileLength(SAVE_FILE)));
            }
            else TextCopy(status, "no save file: press S first");
        }

        if (IsKeyPressed(KEY_D))
        {
            if (FileExists(SAVE_FILE)) { remove(SAVE_FILE); TextCopy(status, "save deleted"); }
            TextCopy(loaded, "(not loaded)");
        }

        // ---- CLIPBOARD --------------------------------------------------------
        if (IsKeyPressed(KEY_C))
        {
            SetClipboardText(TextFormat("raylib score: %i", score));
            TextCopy(status, "copied to the system clipboard");
        }

        // ---- COMPRESSION ------------------------------------------------------
        if (IsKeyPressed(KEY_Z))
        {
            unsigned char raw[2048];
            for (int i = 0; i < 2048; i++) raw[i] = (unsigned char)(i % 16);   // very compressible
            rawSize = 2048;

            unsigned char *packed = CompressData(raw, rawSize, &compressedSize);
            MemFree(packed);            // raylib's own free, pairs with its allocs
            TextCopy(status, "compressed a 2 KB test buffer");
        }

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            int y = 16;
            DrawText("rcore - FILES, RANDOM, MISC", 20, y, 24, DARKGRAY); y += 40;

            // ---- file system (string work only) -------------------------------
            DrawText("FILE SYSTEM (these only parse strings)", 20, y, 18, MAROON); y += 24;
            const char *path = "levels/world1/boss.level.png";
            DrawText(TextFormat("path                 %s", path), 30, y, 15, BLACK); y += 19;
            DrawText(TextFormat("GetFileName()        %s", GetFileName(path)), 30, y, 15, BLACK); y += 19;
            DrawText(TextFormat("GetFileExtension()   %s", GetFileExtension(path)), 30, y, 15, BLACK); y += 19;
            DrawText(TextFormat("GetFileNameWithoutExt()  %s", GetFileNameWithoutExt(path)), 30, y, 15, BLACK); y += 19;
            DrawText(TextFormat("GetDirectoryPath()   %s", GetDirectoryPath(path)), 30, y, 15, BLACK); y += 19;
            DrawText(TextFormat("IsFileExtension(.png)  %s", IsFileExtension(path, ".png") ? "yes" : "no"), 30, y, 15, BLACK); y += 26;

            DrawText(TextFormat("GetWorkingDirectory()    %s", GetWorkingDirectory()), 30, y, 15, DARKBLUE); y += 19;
            DrawText(TextFormat("GetApplicationDirectory() %s", GetApplicationDirectory()), 30, y, 15, DARKBLUE); y += 30;

            // ---- random --------------------------------------------------------
            DrawText("RANDOM", 20, y, 18, MAROON); y += 24;
            DrawText(TextFormat("GetRandomValue(1,6) x6:  %i %i %i %i %i %i",
                     dice[0], dice[1], dice[2], dice[3], dice[4], dice[5]), 30, y, 16, BLACK); y += 22;
            DrawText(TextFormat("LoadRandomSequence(10,1,10): %i %i %i %i %i %i %i %i %i %i  (no repeats)",
                     deck[0], deck[1], deck[2], deck[3], deck[4],
                     deck[5], deck[6], deck[7], deck[8], deck[9]), 30, y, 16, BLACK); y += 30;

            // ---- file data ------------------------------------------------------
            DrawText("FILE DATA", 20, y, 18, MAROON); y += 24;
            DrawText(TextFormat("%s exists: %s", SAVE_FILE, FileExists(SAVE_FILE) ? "YES" : "no"), 30, y, 16, BLACK); y += 22;

            // Draw the loaded text line by line
            {
                int lineCount = 0;
                char **lines = TextSplit(loaded, '\n', &lineCount);
                for (int i = 0; i < lineCount && i < 4; i++)
                {
                    DrawText(TextFormat("  %s", lines[i]), 30, y, 15, DARKGREEN);
                    y += 18;
                }
            }
            y += 12;

            // ---- misc ------------------------------------------------------------
            DrawText("MISC", 20, y, 18, MAROON); y += 24;
            if (rawSize > 0)
            {
                DrawText(TextFormat("CompressData: %i bytes -> %i bytes (%.0f%%)",
                         rawSize, compressedSize, 100.0f*compressedSize/rawSize), 30, y, 16, BLACK);
                y += 22;
            }
            const char *clip = GetClipboardText();
            DrawText(TextFormat("clipboard: %s", (clip && clip[0]) ? clip : "(empty)"), 30, y, 15, DARKPURPLE); y += 26;

            DrawRectangle(20, y, 940, 26, Fade(ORANGE, 0.25f));
            DrawText(TextFormat("status: %s", status), 28, y + 5, 16, DARKGRAY);

            DrawRectangle(0, GetScreenHeight() - 28, GetScreenWidth(), 28, Fade(BLACK, 0.8f));
            DrawText("R random   S save   L load   D delete save   C clipboard   Z compress",
                     10, GetScreenHeight() - 21, 15, RAYWHITE);
        EndDrawing();
    }

    UnloadRandomSequence(deck);
    CloseWindow();
    return 0;
}
