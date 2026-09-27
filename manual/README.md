# raylib from Zero — the manual

`raylib-from-zero.pdf` is an 87 page manual that takes you from an empty `main()`
to a small first-person shooter, and then walks the rest of the library module by
module. It is written for someone who has never worked in 3D before.

Every chapter has a matching program in [`../course2d/`](../course2d),
[`../course3d/`](../course3d) or [`../cheatsheet/`](../cheatsheet), and every line
of code printed in the manual comes from one of those files, so it compiles and
runs.

## Part I — Making a game

| Chapters | Topic |
|---|---|
| 1–2 | how a raylib program is shaped: the frame loop, `BeginDrawing`/`EndDrawing`, immediate mode |
| 3–5 | the screen, colours, shapes, input, and delta time |
| 6–8 | 2D collisions, textures and sprites, `Camera2D` and world vs screen space |
| 9–11 | the X/Y/Z axes, the 3D camera, drawing 3D objects |
| 12–13 | a first-person camera built by hand, and 3D collisions |
| 14 | building the mini Doom |

Read it in order and you end up with a game.

## Part II — The rest of the library

| Chapter | Module | Topic |
|---|---|---|
| 15 | raudio | sound and music, including synthesising both in C |
| 16 | rcore | shaders, and why your cubes look flat |
| 17 | rmodels | meshes, models, materials, billboards, animation |
| 18 | rtextures | image editing, generation, render textures, colour |
| 19 | rtext | fonts, measuring, string helpers, UTF-8 |
| 20 | rcore | saving and loading, the window, gamepads, gestures |
| 21 | raymath | vectors, dot and cross products, matrices, quaternions |
| 22 | — | where to go from here |

Each chapter stands alone: read the one you need when you need it.

Appendices: a cheat sheet, a troubleshooting guide, and an index of all 23
example programs with their controls.

## Rebuilding the PDF

Needs `pdflatex` with `listings`, `mdframed`, `tikz`, `titlesec`, `fancyhdr`,
`hyperref`, `booktabs` and `amsmath`. Run it twice so the table of contents
resolves:

```bash
cd manual
pdflatex raylib-from-zero.tex
pdflatex raylib-from-zero.tex
```

The chapters live in [`parts/`](parts) as one `.tex` file each; the master file
only holds the preamble and the list of `\input` lines.
