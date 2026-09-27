/* =============================================================================
   CHEATSHEET MODULE 6/10 - rtextures: images, textures, render textures, colour
   -----------------------------------------------------------------------------
   The cheatsheet splits this module into five blocks, and the split matters:

     Image loading          pixels in RAM (CPU)
     Image manipulation     resize, crop, flip, recolour... CPU work, slow, free
     Image generation       make pixels from nothing: gradients, noise, checks
     Texture loading/draw   the same pixels on the GPU. Fast to draw, fixed.
     Colour/pixel helpers   Fade, ColorTint, ColorToHSV and friends

   The rule of thumb: do all your Image work first, upload once, then only draw
   the Texture. Going back the other way (LoadImageFromTexture) means reading
   from the GPU, which is slow enough that you should not do it per frame.

   A RenderTexture is the third thing: a texture you can DRAW INTO. That is how
   you make minimaps, mirrors, and pixel-art games that render small and scale up.

   Controls: 1-6 change the generated image, T cycles the texture filter,
             SPACE toggles the render texture view.
   ============================================================================= */

#include "raylib.h"
#include <math.h>

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1080, 700, "Module 6 - rtextures");
    SetTargetFPS(60);

    // =========================================================================
    // IMAGE GENERATION - pixels from nothing, no files needed
    // =========================================================================
    Image images[6] = {
        GenImageColor(256, 256, SKYBLUE),
        GenImageGradientLinear(256, 256, 0, RED, YELLOW),
        GenImageGradientRadial(256, 256, 0.3f, WHITE, DARKBLUE),
        GenImageChecked(256, 256, 32, 32, DARKGRAY, RAYWHITE),
        GenImageWhiteNoise(256, 256, 0.4f),
        GenImagePerlinNoise(256, 256, 0, 0, 4.0f)
    };
    const char *imageNames[] = { "GenImageColor", "GenImageGradientLinear",
                                 "GenImageGradientRadial", "GenImageChecked",
                                 "GenImageWhiteNoise", "GenImagePerlinNoise" };
    int current = 3;

    // =========================================================================
    // IMAGE MANIPULATION - all of this happens on the CPU, before uploading
    // =========================================================================
    Image edited = ImageCopy(images[3]);        // never edit the original
    ImageResize(&edited, 128, 128);             // scale
    ImageCrop(&edited, (Rectangle){ 0, 0, 128, 96 });
    ImageFlipHorizontal(&edited);
    ImageColorTint(&edited, ORANGE);            // multiply every pixel
    ImageColorBrightness(&edited, 30);
    ImageDrawCircle(&edited, 64, 48, 24, Fade(BLACK, 0.5f));   // draw INTO the image
    ImageDrawText(&edited, "CPU", 46, 40, 20, RAYWHITE);

    // =========================================================================
    // TEXTURES - the same pixels, uploaded to the GPU
    // =========================================================================
    Texture2D textures[6];
    for (int i = 0; i < 6; i++) textures[i] = LoadTextureFromImage(images[i]);

    Texture2D editedTex = LoadTextureFromImage(edited);

    // Mipmaps + a trilinear filter make a texture look right when it is drawn
    // much smaller than its real size. Without them you get shimmering.
    GenTextureMipmaps(&textures[3]);

    int filter = 0;
    const char *filterNames[] = { "POINT (crisp, pixel-art)", "BILINEAR (smooth)", "TRILINEAR (+ mipmaps)" };

    // =========================================================================
    // RENDER TEXTURE - a texture you draw into
    // =========================================================================
    RenderTexture2D canvas = LoadRenderTexture(320, 240);
    bool showCanvas = true;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ONE))   current = 0;
        if (IsKeyPressed(KEY_TWO))   current = 1;
        if (IsKeyPressed(KEY_THREE)) current = 2;
        if (IsKeyPressed(KEY_FOUR))  current = 3;
        if (IsKeyPressed(KEY_FIVE))  current = 4;
        if (IsKeyPressed(KEY_SIX))   current = 5;
        if (IsKeyPressed(KEY_SPACE)) showCanvas = !showCanvas;

        if (IsKeyPressed(KEY_T))
        {
            filter = (filter + 1) % 3;
            SetTextureFilter(textures[current],
                filter == 0 ? TEXTURE_FILTER_POINT :
                filter == 1 ? TEXTURE_FILTER_BILINEAR : TEXTURE_FILTER_TRILINEAR);
        }

        // ---- DRAW INTO THE RENDER TEXTURE -----------------------------------
        // Its own Begin/End pair, and it must come BEFORE BeginDrawing.
        BeginTextureMode(canvas);
            ClearBackground((Color){ 25, 30, 40, 255 });
            DrawCircle(160 + (int)(60*sinf((float)GetTime()*2.0f)), 120, 34, ORANGE);
            DrawRectangle(20, 20, 60, 40, MAROON);
            DrawText("drawn into a", 20, 180, 20, RAYWHITE);
            DrawText("RenderTexture", 20, 202, 20, RAYWHITE);
        EndTextureMode();

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            DrawText("rtextures", 20, 14, 24, DARKGRAY);

            // ---- generated image, as a texture ------------------------------
            DrawText(TextFormat("1-6  %s", imageNames[current]), 20, 52, 17, MAROON);
            DrawTexture(textures[current], 20, 78, WHITE);
            DrawRectangleLines(20, 78, 256, 256, LIGHTGRAY);

            // ---- the CPU-edited image --------------------------------------
            DrawText("ImageResize/Crop/Flip/Tint/Draw", 300, 52, 17, MAROON);
            DrawTexture(editedTex, 300, 78, WHITE);
            DrawRectangleLines(300, 78, editedTex.width, editedTex.height, LIGHTGRAY);

            // ---- the four ways to draw a texture ----------------------------
            DrawText("DrawTexture variants", 300, 200, 17, MAROON);
            DrawTextureV(editedTex, (Vector2){ 300, 226 }, Fade(WHITE, 0.6f));
            DrawText("V + Fade tint", 300, 330, 13, GRAY);

            DrawTextureEx(editedTex, (Vector2){ 460, 226 }, 15.0f, 0.6f, WHITE);
            DrawText("Ex: rotated + scaled", 460, 330, 13, GRAY);

            // Source rectangle: draw only a PIECE. Negative width flips it.
            DrawTextureRec(editedTex, (Rectangle){ 0, 0, 64, 96 }, (Vector2){ 640, 226 }, WHITE);
            DrawText("Rec: left half", 640, 330, 13, GRAY);

            DrawTexturePro(editedTex,
                           (Rectangle){ 0, 0, (float)editedTex.width, (float)editedTex.height },
                           (Rectangle){ 800, 270, 100, 75 },
                           (Vector2){ 50, 37 }, (float)(GetTime()*40.0), SKYBLUE);
            DrawText("Pro: origin + rotation + tint", 740, 330, 13, GRAY);

            DrawText(TextFormat("T  filter: %s", filterNames[filter]), 300, 352, 15, DARKBLUE);

            // ---- the render texture -----------------------------------------
            if (showCanvas)
            {
                DrawText("RenderTexture2D (SPACE to hide)", 20, 380, 17, MAROON);

                // IMPORTANT: a render texture is stored upside down, so the
                // source rectangle needs a NEGATIVE height to flip it back.
                DrawTexturePro(canvas.texture,
                               (Rectangle){ 0, 0, (float)canvas.texture.width, -(float)canvas.texture.height },
                               (Rectangle){ 20, 406, 320, 240 },
                               (Vector2){ 0, 0 }, 0.0f, WHITE);
                DrawRectangleLines(20, 406, 320, 240, LIGHTGRAY);
                DrawText("note the NEGATIVE source height:", 360, 410, 15, DARKPURPLE);
                DrawText("render textures come out upside down", 360, 430, 15, DARKPURPLE);
            }

            // ---- colour helpers ---------------------------------------------
            DrawText("COLOUR HELPERS", 360, 470, 17, MAROON);
            Color base = MAROON;
            int cx = 360;
            DrawRectangle(cx, 496, 60, 46, base);                       DrawText("base", cx, 546, 12, GRAY); cx += 70;
            DrawRectangle(cx, 496, 60, 46, Fade(base, 0.4f));           DrawText("Fade .4", cx, 546, 12, GRAY); cx += 70;
            DrawRectangle(cx, 496, 60, 46, ColorTint(base, SKYBLUE));   DrawText("Tint", cx, 546, 12, GRAY); cx += 70;
            DrawRectangle(cx, 496, 60, 46, ColorBrightness(base, 0.5f));DrawText("Brighter", cx, 546, 12, GRAY); cx += 70;
            DrawRectangle(cx, 496, 60, 46, ColorContrast(base, 0.7f));  DrawText("Contrast", cx, 546, 12, GRAY); cx += 70;

            Vector3 hsv = ColorToHSV(base);
            DrawRectangle(cx, 496, 60, 46, ColorFromHSV(hsv.x + 120, hsv.y, hsv.z));
            DrawText("HSV +120", cx, 546, 12, GRAY);

            DrawText(TextFormat("ColorToHSV(MAROON) = h %.0f  s %.2f  v %.2f", hsv.x, hsv.y, hsv.z),
                     360, 572, 15, DARKBLUE);
            DrawText(TextFormat("ColorToInt(MAROON) = 0x%08X", ColorToInt(base)), 360, 592, 15, DARKBLUE);

            DrawRectangle(0, GetScreenHeight() - 26, GetScreenWidth(), 26, Fade(BLACK, 0.8f));
            DrawText("1-6 generated image   T texture filter   SPACE render texture",
                     10, GetScreenHeight() - 20, 15, RAYWHITE);
        EndDrawing();
    }

    // ---- UNLOAD everything --------------------------------------------------
    for (int i = 0; i < 6; i++) { UnloadTexture(textures[i]); UnloadImage(images[i]); }
    UnloadTexture(editedTex);
    UnloadImage(edited);
    UnloadRenderTexture(canvas);

    CloseWindow();
    return 0;
}
