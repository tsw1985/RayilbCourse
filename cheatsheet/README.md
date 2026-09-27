# The cheatsheet, module by module

One runnable example per module of the [official raylib cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html).
The cheatsheet organises the library into six modules plus `raymath`; `rcore` is
large enough that it is split over four files here.

```bash
./cheatsheet/build.sh 1     # and 2 … 10
```

Nothing loads an external asset. Every image, sound and font is generated in
code, so each file compiles and runs on its own.

| # | File | Cheatsheet module | What it demonstrates |
|---|---|---|---|
| 1 | `m1_rcore_window.c` | rcore | window state, flags, monitors, cursor, timing, screenshots |
| 2 | `m2_rcore_input.c` | rcore | keyboard, mouse, gamepad, touch, gestures, dropped files |
| 3 | `m3_rcore_shaders.c` | rcore | GLSL from memory, uniforms, `BeginShaderMode` |
| 4 | `m4_rcore_misc.c` | rcore | file system, reading/writing files, random, log, clipboard, compression |
| 5 | `m5_rshapes.c` | rshapes | every 2D shape, splines, and the collision tests |
| 6 | `m6_rtextures.c` | rtextures | `Image` vs `Texture`, generation, CPU editing, render textures, colour helpers |
| 7 | `m7_rtext.c` | rtext | fonts, measuring, text utilities, UTF-8 codepoints |
| 8 | `m8_rmodels.c` | rmodels | mesh generation, models, materials, billboards, ray picking |
| 9 | `m9_raudio.c` | raudio | synthesised sounds, streamed music, live audio streams |
| 10 | `m10_raymath.c` | raymath | vectors, dot and cross products, lerp, matrices, quaternions |

## Coverage

These ten files call **225 of the 619 functions** in `raylib.h` (36%). Together
with the course lessons, the code in this repository exercises **240 functions,
39% of the library**.

What is still untouched is mostly depth rather than breadth: the remaining
`Image*` manipulation calls, model animation, audio stream buffer management,
VR, and the `rlgl` low-level layer, which has its own header.

## A note on versions

These examples are written against the **raylib 6.1-dev** source vendored in
`build/external/raylib-master/`, which is what this project links. The website's
cheatsheet documents the latest *stable release*, so a few signatures differ.
One that bit during writing:

```c
// 5.5 (and the website cheatsheet)
DrawCircleGradient(int centerX, int centerY, float radius, Color inner, Color outer);

// 6.1-dev (what you actually link against)
DrawCircleGradient(Vector2 center, float radius, Color inner, Color outer);
```

When the two disagree, `build/external/raylib-master/src/raylib.h` wins.
