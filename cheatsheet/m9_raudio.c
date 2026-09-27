/* =============================================================================
   CHEATSHEET MODULE 9/10 - raudio: sounds, music and audio streams
   -----------------------------------------------------------------------------
   raudio has four blocks, and the difference between them is WHERE the samples
   live and WHO pushes them:

     Sound         short clip, fully decoded in RAM. Play it, forget it.
                   Gunshots, footsteps, UI clicks.
     Music         long track, streamed from disk a chunk at a time. You MUST
                   call UpdateMusicStream() every frame or it stops.
     AudioStream   raw samples that YOU generate and push. Synthesisers,
                   voice chat, anything procedural.
     Wave          the samples themselves, on the CPU. A Sound is a Wave that
                   has been uploaded and is ready to play.

   Nothing here needs an audio file: this example SYNTHESISES every sound in C,
   and writes a .wav to disk to demonstrate the Music path.

   Controls: 1 beep, 2 laser, 3 explosion, M play/pause music,
             SPACE hold for the live stream, UP/DOWN volume.
   ============================================================================= */

#include "raylib.h"
#include <math.h>

#define SAMPLE_RATE 44100

// ---------------------------------------------------------------------------
// Build a Wave in memory. 16-bit mono is the simplest useful format:
// one 'short' per frame, -32768..32767.
// ---------------------------------------------------------------------------
static Wave MakeWave(float seconds, float startHz, float endHz, float noiseAmount)
{
    Wave wave = { 0 };
    wave.sampleRate = SAMPLE_RATE;
    wave.sampleSize = 16;               // bits per sample
    wave.channels   = 1;                // mono
    wave.frameCount = (unsigned int)(seconds*SAMPLE_RATE);

    // MemAlloc is raylib's allocator: UnloadWave will free it with the matching one
    short *samples = (short *)MemAlloc(wave.frameCount*sizeof(short));

    float phase = 0.0f;
    unsigned int seed = 12345;

    for (unsigned int i = 0; i < wave.frameCount; i++)
    {
        float t = (float)i/(float)wave.frameCount;      // 0..1 through the sound

        // Pitch slides from startHz to endHz: that is the whole "laser" effect
        float hz = startHz + (endHz - startHz)*t;
        phase += 2.0f*PI*hz/SAMPLE_RATE;

        float value = sinf(phase);

        if (noiseAmount > 0.0f)
        {
            seed = seed*1103515245u + 12345u;           // cheap random
            float noise = ((float)((seed >> 16) & 0x7FFF)/16383.5f) - 1.0f;
            value = value*(1.0f - noiseAmount) + noise*noiseAmount;
        }

        // Envelope: fade out over the length of the sound so it does not click
        float envelope = (1.0f - t)*(1.0f - t);

        samples[i] = (short)(value*envelope*22000.0f);
    }

    wave.data = samples;
    return wave;
}

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(980, 620, "Module 9 - raudio");
    SetTargetFPS(60);

    // Nothing in raudio works before this line, exactly like InitWindow.
    InitAudioDevice();

    // =========================================================================
    // SOUNDS - synthesised, then uploaded
    // =========================================================================
    Wave beepWave  = MakeWave(0.18f, 880.0f,  880.0f, 0.0f);   // a plain tone
    Wave laserWave = MakeWave(0.30f, 1400.0f, 200.0f, 0.05f);  // falling pitch
    Wave boomWave  = MakeWave(0.60f, 120.0f,   40.0f, 0.85f);  // mostly noise

    Sound beep  = LoadSoundFromWave(beepWave);
    Sound laser = LoadSoundFromWave(laserWave);
    Sound boom  = LoadSoundFromWave(boomWave);

    // =========================================================================
    // MUSIC - written to a real .wav, then streamed back off disk
    // =========================================================================
    Wave tuneWave = { 0 };
    tuneWave.sampleRate = SAMPLE_RATE;
    tuneWave.sampleSize = 16;
    tuneWave.channels = 1;
    tuneWave.frameCount = SAMPLE_RATE*4;                        // 4 seconds
    short *tune = (short *)MemAlloc(tuneWave.frameCount*sizeof(short));
    {
        const float notes[8] = { 262, 294, 330, 349, 392, 440, 494, 523 };   // a C major scale
        float phase = 0.0f;
        for (unsigned int i = 0; i < tuneWave.frameCount; i++)
        {
            int note = (int)((float)i/tuneWave.frameCount*8.0f);
            if (note > 7) note = 7;
            phase += 2.0f*PI*notes[note]/SAMPLE_RATE;
            tune[i] = (short)(sinf(phase)*9000.0f);
        }
    }
    tuneWave.data = tune;

    ExportWave(tuneWave, "generated_tune.wav");     // raudio can write files too
    UnloadWave(tuneWave);

    Music music = LoadMusicStream("generated_tune.wav");
    music.looping = true;
    bool musicPlaying = false;

    // =========================================================================
    // AUDIO STREAM - samples you push yourself, live
    // =========================================================================
    AudioStream stream = LoadAudioStream(SAMPLE_RATE, 16, 1);
    PlayAudioStream(stream);
    float streamPhase = 0.0f;
    bool streamOn = false;

    float masterVolume = 0.6f;
    SetMasterVolume(masterVolume);

    while (!WindowShouldClose())
    {
        // ---- SOUNDS -----------------------------------------------------
        if (IsKeyPressed(KEY_ONE))   PlaySound(beep);
        if (IsKeyPressed(KEY_TWO))
        {
            // Vary pitch and pan per shot so repeats do not sound identical
            SetSoundPitch(laser, 0.85f + GetRandomValue(0, 30)/100.0f);
            SetSoundPan(laser, GetRandomValue(20, 80)/100.0f);
            PlaySound(laser);
        }
        if (IsKeyPressed(KEY_THREE)) PlaySound(boom);

        // ---- MUSIC -------------------------------------------------------
        if (IsKeyPressed(KEY_M))
        {
            if (musicPlaying) { PauseMusicStream(music); musicPlaying = false; }
            else { if (GetMusicTimePlayed(music) <= 0.0f) PlayMusicStream(music);
                   else ResumeMusicStream(music);
                   musicPlaying = true; }
        }

        // THE line people forget. Without it the music plays for a fraction of
        // a second and stops: the buffer is never refilled.
        if (musicPlaying) UpdateMusicStream(music);

        // ---- AUDIO STREAM -------------------------------------------------
        streamOn = IsKeyDown(KEY_SPACE);

        if (IsAudioStreamProcessed(stream))
        {
            // The device has finished a buffer and wants the next one
            static short chunk[4096];
            float hz = 220.0f + GetMousePosition().y;      // mouse controls pitch

            for (int i = 0; i < 4096; i++)
            {
                streamPhase += 2.0f*PI*hz/SAMPLE_RATE;
                chunk[i] = streamOn ? (short)(sinf(streamPhase)*8000.0f) : 0;
            }

            UpdateAudioStream(stream, chunk, 4096);
        }

        // ---- VOLUME --------------------------------------------------------
        if (IsKeyDown(KEY_UP))   masterVolume += 0.4f*GetFrameTime();
        if (IsKeyDown(KEY_DOWN)) masterVolume -= 0.4f*GetFrameTime();
        if (masterVolume < 0.0f) masterVolume = 0.0f;
        if (masterVolume > 1.0f) masterVolume = 1.0f;
        SetMasterVolume(masterVolume);

        BeginDrawing();
            ClearBackground((Color){ 245, 245, 245, 255 });

            int y = 16;
            DrawText("raudio", 20, y, 24, DARKGRAY); y += 40;

            DrawText(TextFormat("audio device ready: %s", IsAudioDeviceReady() ? "YES" : "NO"), 20, y, 17,
                     IsAudioDeviceReady() ? DARKGREEN : RED); y += 34;

            // ---- Sound ------------------------------------------------------
            DrawText("SOUND - short, decoded in RAM, fire and forget", 20, y, 18, MAROON); y += 26;
            DrawText(TextFormat("1  beep       %s", IsSoundPlaying(beep) ? "PLAYING" : "idle"), 34, y, 16, BLACK); y += 22;
            DrawText(TextFormat("2  laser      %s   (random pitch + pan each shot)",
                     IsSoundPlaying(laser) ? "PLAYING" : "idle"), 34, y, 16, BLACK); y += 22;
            DrawText(TextFormat("3  explosion  %s", IsSoundPlaying(boom) ? "PLAYING" : "idle"), 34, y, 16, BLACK); y += 22;
            DrawText("all three were synthesised in C: no .wav was loaded", 34, y, 14, GRAY); y += 32;

            // ---- Music -------------------------------------------------------
            DrawText("MUSIC - long, streamed from disk, needs UpdateMusicStream()", 20, y, 18, MAROON); y += 26;
            DrawText(TextFormat("M  generated_tune.wav   %s", musicPlaying ? "PLAYING" : "paused"), 34, y, 16, BLACK); y += 22;
            {
                float length = GetMusicTimeLength(music);
                float played = GetMusicTimePlayed(music);
                DrawText(TextFormat("%.1f / %.1f s", played, length), 34, y, 16, BLACK);
                DrawRectangle(160, y, 400, 14, LIGHTGRAY);
                if (length > 0.0f) DrawRectangle(160, y, (int)(400.0f*played/length), 14, DARKGREEN);
                y += 30;
            }

            // ---- AudioStream --------------------------------------------------
            DrawText("AUDIOSTREAM - samples you generate and push yourself", 20, y, 18, MAROON); y += 26;
            DrawText(TextFormat("SPACE (hold)  %s     mouse Y sets the pitch: %.0f Hz",
                     streamOn ? "TONE ON" : "silent", 220.0f + GetMousePosition().y), 34, y, 16,
                     streamOn ? DARKGREEN : BLACK); y += 32;

            // ---- Volume --------------------------------------------------------
            DrawText(TextFormat("UP/DOWN  master volume %.2f", masterVolume), 20, y, 17, DARKBLUE); y += 24;
            DrawRectangle(20, y, 300, 16, LIGHTGRAY);
            DrawRectangle(20, y, (int)(300*masterVolume), 16, DARKBLUE);

            DrawRectangle(0, GetScreenHeight() - 26, GetScreenWidth(), 26, Fade(BLACK, 0.8f));
            DrawText("1 beep   2 laser   3 boom   M music   SPACE stream   UP/DOWN volume",
                     10, GetScreenHeight() - 20, 15, RAYWHITE);
        EndDrawing();
    }

    // ---- UNLOAD, then close the device, then the window ---------------------
    UnloadSound(beep);
    UnloadSound(laser);
    UnloadSound(boom);
    UnloadWave(beepWave);       // the Wave and the Sound are separate allocations
    UnloadWave(laserWave);
    UnloadWave(boomWave);
    UnloadMusicStream(music);
    UnloadAudioStream(stream);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
