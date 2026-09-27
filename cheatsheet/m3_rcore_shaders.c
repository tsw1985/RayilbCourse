/* =============================================================================
   CHEATSHEET MODULE 3/10 - rcore: shaders
   -----------------------------------------------------------------------------
   A shader is a small program that runs ON THE GPU, once per vertex or once per
   pixel. It is the only way to do real lighting, and the reason plain DrawCube
   looks flat.

   You get two of them:

     VERTEX shader    runs once per corner. Decides WHERE things land on screen.
     FRAGMENT shader  runs once per pixel. Decides WHAT COLOUR that pixel is.

   raylib gives you a default pair, so you only have to write the one you care
   about and pass NULL for the other.

   The three steps are always the same:

     1. LoadShader / LoadShaderFromMemory    (once, outside the loop)
     2. GetShaderLocation("uniformName")     (once: find the variable's slot)
     3. SetShaderValue(...) then Begin/EndShaderMode  (every frame)

   A "uniform" is just a variable you send from C to the GPU program.

   Controls: 1/2/3 switch shader, mouse moves the light, SPACE pauses time.
   ============================================================================= */

#include "raylib.h"
#include <stddef.h>

// GLSL 3.30 is what raylib uses on desktop. Note the "#version" line must be
// the very first thing in the string, with nothing before it.
static const char *fsGrayscale =
"#version 330\n"
"in vec2 fragTexCoord;\n"
"in vec4 fragColor;\n"
"uniform sampler2D texture0;\n"
"uniform vec4 colDiffuse;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"    vec4 texel = texture(texture0, fragTexCoord)*colDiffuse*fragColor;\n"
"    float grey = dot(texel.rgb, vec3(0.299, 0.587, 0.114));\n"
"    finalColor = vec4(grey, grey, grey, texel.a);\n"
"}\n";

static const char *fsWave =
"#version 330\n"
"in vec2 fragTexCoord;\n"
"in vec4 fragColor;\n"
"uniform sampler2D texture0;\n"
"uniform vec4 colDiffuse;\n"
"uniform float uTime;\n"            // <- our own uniform, sent from C
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"    vec2 uv = fragTexCoord;\n"
"    uv.x += sin(uv.y*18.0 + uTime*3.0)*0.02;\n"
"    finalColor = texture(texture0, uv)*colDiffuse*fragColor;\n"
"}\n";

static const char *fsSpotlight =
"#version 330\n"
"in vec2 fragTexCoord;\n"
"in vec4 fragColor;\n"
"uniform sampler2D texture0;\n"
"uniform vec4 colDiffuse;\n"
"uniform vec2 uLight;\n"            // light position, in pixels
"uniform vec2 uResolution;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"    vec4 texel = texture(texture0, fragTexCoord)*colDiffuse*fragColor;\n"
"    vec2 pixel = fragTexCoord*uResolution;\n"
"    float d = distance(pixel, uLight);\n"
"    float light = clamp(1.0 - d/320.0, 0.08, 1.0);\n"
"    finalColor = vec4(texel.rgb*light, texel.a);\n"
"}\n";

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(960, 620, "Module 3 - rcore: shaders");
    SetTargetFPS(60);

    // ---- 1. LOAD, once -------------------------------------------------------
    // LoadShader(vsFile, fsFile) reads from disk.
    // LoadShaderFromMemory(vsCode, fsCode) takes the source as strings.
    // NULL for either one means "use raylib's default".
    Shader grayscale = LoadShaderFromMemory(NULL, fsGrayscale);
    Shader wave      = LoadShaderFromMemory(NULL, fsWave);
    Shader spotlight = LoadShaderFromMemory(NULL, fsSpotlight);

    // ---- 2. FIND THE UNIFORM SLOTS, once ------------------------------------
    // This is a lookup by name into the compiled program. Doing it every frame
    // works but is wasteful; do it once and keep the int.
    int locTime       = GetShaderLocation(wave, "uTime");
    int locLight      = GetShaderLocation(spotlight, "uLight");
    int locResolution = GetShaderLocation(spotlight, "uResolution");

    float resolution[2] = { (float)GetScreenWidth(), (float)GetScreenHeight() };
    SetShaderValue(spotlight, locResolution, resolution, SHADER_UNIFORM_VEC2);

    // Something to apply the shaders to: a generated texture
    Image checked = GenImageChecked(512, 512, 32, 32, (Color){ 60, 90, 160, 255 }, RAYWHITE);
    Texture2D tex = LoadTextureFromImage(checked);
    UnloadImage(checked);

    int current = 0;
    const char *names[] = { "none (default shader)", "grayscale", "wave (uTime uniform)", "spotlight (uLight uniform)" };
    float shaderTime = 0.0f;
    bool paused = false;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ZERO) || IsKeyPressed(KEY_KP_0)) current = 0;
        if (IsKeyPressed(KEY_ONE))   current = 1;
        if (IsKeyPressed(KEY_TWO))   current = 2;
        if (IsKeyPressed(KEY_THREE)) current = 3;
        if (IsKeyPressed(KEY_SPACE)) paused = !paused;

        if (!paused) shaderTime += GetFrameTime();

        // ---- 3. FEED THE UNIFORMS, every frame ------------------------------
        SetShaderValue(wave, locTime, &shaderTime, SHADER_UNIFORM_FLOAT);

        Vector2 m = GetMousePosition();
        float light[2] = { m.x, (float)GetScreenHeight() - m.y };   // GL's Y is flipped
        SetShaderValue(spotlight, locLight, light, SHADER_UNIFORM_VEC2);

        BeginDrawing();
            ClearBackground((Color){ 20, 22, 28, 255 });

            // Everything between BeginShaderMode and EndShaderMode is drawn
            // through that shader. Same Begin/End contract as the cameras.
            if (current == 1) BeginShaderMode(grayscale);
            else if (current == 2) BeginShaderMode(wave);
            else if (current == 3) BeginShaderMode(spotlight);

                DrawTexture(tex, 220, 60, WHITE);
                DrawCircle(480, 300, 90, Fade(ORANGE, 0.85f));
                DrawText("SHADED", 360, 280, 60, RAYWHITE);

            if (current != 0) EndShaderMode();

            // Outside the pair: normal drawing, untouched by the shader
            DrawRectangle(0, 0, GetScreenWidth(), 46, Fade(BLACK, 0.75f));
            DrawText(TextFormat("shader: %s", names[current]), 16, 14, 20, RAYWHITE);

            DrawRectangle(0, GetScreenHeight() - 52, GetScreenWidth(), 52, Fade(BLACK, 0.75f));
            DrawText("0 none   1 grayscale   2 wave   3 spotlight   SPACE pause", 14, GetScreenHeight() - 44, 16, RAYWHITE);
            DrawText(TextFormat("uTime = %.2f   uLight = %.0f, %.0f   (this HUD is outside the shader)",
                     shaderTime, light[0], light[1]), 14, GetScreenHeight() - 22, 15, GRAY);
        EndDrawing();
    }

    // ---- UNLOAD --------------------------------------------------------------
    UnloadShader(grayscale);
    UnloadShader(wave);
    UnloadShader(spotlight);
    UnloadTexture(tex);

    CloseWindow();
    return 0;
}
