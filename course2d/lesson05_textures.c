/* =============================================================================
   LESSON 05 - Images, textures and sprites
   -----------------------------------------------------------------------------
   Two different things that beginners mix up constantly:

     Image    pixels living in normal RAM (CPU). You can read and edit them,
              but you cannot draw them. Cheap to modify, useless to display.

     Texture  the same pixels uploaded to the GPU (VRAM). You can draw it very
              fast, but you cannot poke individual pixels any more.

   The usual flow is: load/build an Image, upload it once with
   LoadTextureFromImage(), free the Image, then draw the Texture every frame.

   THE GOLDEN RULE OF LOADING: load OUTSIDE the loop, unload AFTER the loop.
   Calling LoadTexture() inside the frame loop uploads a new copy to the GPU
   sixty times a second and your memory is gone in under a minute.

   DrawTextureRec lets you draw just a PIECE of a texture. That is how sprite
   sheets work: one image holding many frames, and you pick the frame with a
   source rectangle. That is the whole trick behind 2D animation.

   Controls: SPACE pauses the animation, arrows change the animation speed.
   ============================================================================= */

#include "raylib.h"

#define FRAME_COUNT 6
#define FRAME_SIZE  64

// Builds a sprite sheet in memory: 6 frames of a bouncing ball, side by side.
// Normally you would load a .png an artist made; we generate one so this lesson
// has no external files.
static Texture2D BuildSpriteSheet(void)
{
    // GenImageColor creates an Image (CPU side) filled with one color
    Image sheet = GenImageColor(FRAME_SIZE*FRAME_COUNT, FRAME_SIZE, BLANK);

    for (int i = 0; i < FRAME_COUNT; i++)
    {
        int cx = i*FRAME_SIZE + FRAME_SIZE/2;

        // The ball goes up and comes back down across the 6 frames
        int lift = (i < FRAME_COUNT/2) ? i*8 : (FRAME_COUNT - 1 - i)*8;
        int cy = FRAME_SIZE - 16 - lift;

        Color c = (Color){ 40 + i*30, 120, 220 - i*20, 255 };

        ImageDrawCircle(&sheet, cx, cy, 12, c);
        ImageDrawCircle(&sheet, cx - 4, cy - 4, 4, Fade(WHITE, 0.8f));   // highlight
        ImageDrawRectangle(&sheet, i*FRAME_SIZE, FRAME_SIZE - 4, FRAME_SIZE, 4, Fade(BLACK, 0.25f));
    }

    // Upload to the GPU. From here on we draw the Texture, not the Image.
    Texture2D texture = LoadTextureFromImage(sheet);

    // The CPU copy is no longer needed: free it or you leak RAM.
    UnloadImage(sheet);

    return texture;
}

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(900, 520, "Lesson 05 - Textures and sprites");
    SetTargetFPS(60);

    // ---- LOAD ONCE, OUTSIDE THE LOOP ---------------------------------------
    Texture2D sheet = BuildSpriteSheet();

    int   frame = 0;
    float frameTimer = 0.0f;
    float frameDuration = 0.12f;    // seconds each frame stays on screen
    bool  paused = false;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE)) paused = !paused;
        if (IsKeyDown(KEY_UP))   frameDuration -= 0.2f*dt;
        if (IsKeyDown(KEY_DOWN)) frameDuration += 0.2f*dt;
        if (frameDuration < 0.02f) frameDuration = 0.02f;
        if (frameDuration > 0.60f) frameDuration = 0.60f;

        // --- Advancing an animation: a timer, not a frame counter ------------
        // Counting frames ties the animation to the frame rate. Counting seconds
        // does not, exactly like movement in lesson 03.
        if (!paused)
        {
            frameTimer += dt;
            while (frameTimer >= frameDuration)
            {
                frameTimer -= frameDuration;
                frame = (frame + 1) % FRAME_COUNT;
            }
        }

        // The piece of the sheet we want: frame number 'frame'
        Rectangle source = { (float)frame*FRAME_SIZE, 0, FRAME_SIZE, FRAME_SIZE };

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            DrawText("the whole sprite sheet (one texture, six frames):", 40, 30, 18, DARKGRAY);

            // Draw the entire texture, top-left corner at (40, 60)
            DrawTexture(sheet, 40, 60, WHITE);
            DrawRectangleLines(40, 60, sheet.width, sheet.height, LIGHTGRAY);

            // Highlight which frame is playing right now
            DrawRectangleLinesEx((Rectangle){ 40 + source.x, 60, FRAME_SIZE, FRAME_SIZE }, 2, RED);

            DrawText("just one frame, DrawTextureRec:", 40, 190, 18, DARKGRAY);
            DrawTextureRec(sheet, source, (Vector2){ 60, 220 }, WHITE);

            DrawText("same frame, scaled and tinted, DrawTexturePro:", 300, 190, 18, DARKGRAY);
            DrawTexturePro(sheet, source,
                           (Rectangle){ 340, 220, FRAME_SIZE*2.5f, FRAME_SIZE*2.5f },
                           (Vector2){ 0, 0 }, 0.0f, ORANGE);

            DrawRectangle(40, 400, 500, 90, Fade(BLACK, 0.06f));
            DrawText(TextFormat("frame %i of %i", frame + 1, FRAME_COUNT), 56, 414, 18, DARKGRAY);
            DrawText(TextFormat("%.0f ms per frame  (%.1f fps of animation)",
                     frameDuration*1000.0f, 1.0f/frameDuration), 56, 440, 18, DARKGRAY);
            DrawText(paused ? "PAUSED - SPACE to resume   UP/DOWN change speed"
                            : "SPACE pause   UP/DOWN change speed", 56, 466, 14, GRAY);

            DrawFPS(800, 10);
        EndDrawing();
    }

    // ---- UNLOAD AFTER THE LOOP ---------------------------------------------
    UnloadTexture(sheet);

    CloseWindow();
    return 0;
}
