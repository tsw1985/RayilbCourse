# A minimal 3D course with raylib

Six lessons. Each one is **a single `.c` file that builds and runs on its own**. Read
them in order: each adds one idea and nothing more.

## Running a lesson

The library is already built at `bin/Debug/libraylib.a` (if not, run `make` at the root
of the project). Then:

```bash
./course3d/build.sh 1      # lesson 01
./course3d/build.sh 5      # lesson 05
```

If you prefer the project's own build system, copy a lesson over `src/main.c` and run
`make` — the Makefile only compiles that one file.

---

## 1. The axes: X, Y, Z

This is the one thing really worth memorizing:

```
        +Y  up
         |
         |
         +--------- +X  to your right
        /
       /
     +Z  towards you (out of the screen)
```

- **X** — left (−) / right (+)
- **Y** — down (−) / **up (+)**  ← the height
- **Z** — far (−) / near (+)  ← the depth

Two classic traps:

1. **In 2D, Y grew downwards** (they are pixels). In 3D it grows upwards. When you draw
   HUD outside `BeginMode3D` you are back in pixel land, with Y the other way round.
2. **The floor is the XZ plane, at height Y = 0.** `DrawGrid()` draws there. An object
   "sitting on the floor" has its *center* at Y = half its height, not at Y = 0.

It is a **right-handed** system: point your right thumb along +X and your index finger
along +Y; your middle finger points along +Z, towards you.

`DrawCube(center, width, height, length, color)` — mind the order: `width` is X, `height`
is Y, `length` is Z. Lesson 1 has three colored cubes so you never have to guess.

**Lesson:** `lesson01_axes.c` — keys `1`/`2`/`3`/`4` view the scene from each axis.

---

## 2. A camera is five pieces of data

```c
Camera3D camera = { 0 };
camera.position   = (Vector3){ 8, 6, 8 };   // where the eye is
camera.target     = (Vector3){ 0, 0, 0 };   // which point it looks at
camera.up         = (Vector3){ 0, 1, 0 };   // what "up" means (almost always +Y)
camera.fovy       = 45.0f;                  // vertical field of view, in degrees
camera.projection = CAMERA_PERSPECTIVE;     // or CAMERA_ORTHOGRAPHIC
```

There is no "direction" field. The direction **is derived**:

```
direction = normalize(target - position)
```

Which gives you the two basic operations:

- **Turn the camera** → move the `target`, leave `position` where it is.
- **Move the camera** → move `position` *and* `target` by the same amount.

About `fovy`: in perspective it is degrees (30 ≈ telephoto, 90 ≈ fisheye, 60–70 is normal
for an FPS). Switch to `CAMERA_ORTHOGRAPHIC` and that same field starts to mean *how many
world units fit vertically* on screen, and perspective disappears (far things no longer
look smaller — this is the look of isometric games and CAD).

And the block that turns it all on:

```c
BeginMode3D(camera);
    // here, 3D WORLD coordinates
EndMode3D();
// here, 2D pixels again: the HUD, the text, the crosshair
```

**Lesson:** `lesson02_camera.c` — move every field with the keyboard and watch the numbers.

---

## 3. The modes raylib already gives you

```c
UpdateCamera(&camera, CAMERA_FREE);
```

| Mode | What for |
|---|---|
| `CAMERA_FREE` | fly around freely, WASD + mouse + wheel |
| `CAMERA_ORBITAL` | spins by itself around the target: inspecting a model |
| `CAMERA_FIRST_PERSON` | first person |
| `CAMERA_THIRD_PERSON` | the camera follows the target from behind |

In first and third person you need `DisableCursor()` to capture the mouse
(`EnableCursor()` to release it).

They are great for prototyping, but the moment you want collisions, crouching, a vehicle
or anything of your own, they fall short. Hence the next lesson.

**Lesson:** `lesson03_modes.c` — `TAB` cycles modes.

---

## 4. Your own first person camera

All it takes is **two angles**:

- `yaw` — horizontal turn, around the **Y** axis (looking sideways)
- `pitch` — vertical turn, around the **X** axis (looking up/down)

From them you get the "forward" vector:

```c
Vector3 forward = {
    cosf(pitch)*cosf(yaw),
    sinf(pitch),
    cosf(pitch)*sinf(yaw)
};
```

And "right" is the cross product with the world's "up":

```c
Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){0,1,0}));
```

With those two you can already move: `W` adds `forward`, `D` adds `right`, and so on. At
the end of the frame, this is what makes the camera look the right way:

```c
camera.target = Vector3Add(camera.position, forward);
```

Four details that separate movement that feels good from movement that feels wrong:

1. **Clamp the `pitch`** to ±89° (±1.55 rad). If you look straight up, `forward` lines up
   with `up`, the cross product becomes zero and the camera snaps. That is *gimbal lock*.
2. **Use a "flat forward"** (`{forward.x, 0, forward.z}` normalized) for walking. With the
   full `forward`, looking at the sky and pressing W makes you take off.
3. **Normalize the movement vector.** Otherwise W+D diagonally moves you 41 % faster (√2).
4. **Always multiply by `GetFrameTime()`.** That way speed is measured in units per
   *second* and does not depend on the machine's frame rate.

**Lesson:** `lesson04_movement.c` — mouse to look, WASD to walk.

---

## 5. The box that notices when you touch it

The simplest 3D collision is the **AABB** (*Axis-Aligned Bounding Box*: a box aligned with
the axes, that never rotates):

```c
typedef struct BoundingBox {
    Vector3 min;   // corner with the smaller coordinates
    Vector3 max;   // corner with the larger coordinates
} BoundingBox;
```

Normally you store your object's **center** and **size**, and compute the box:

```c
BoundingBox BoxFromCenter(Vector3 center, Vector3 size)
{
    BoundingBox b;
    b.min = (Vector3){ center.x - size.x/2, center.y - size.y/2, center.z - size.z/2 };
    b.max = (Vector3){ center.x + size.x/2, center.y + size.y/2, center.z + size.z/2 };
    return b;
}
```

And the check is a single call:

```c
if (CheckCollisionBoxes(boxA, boxB)) { /* they touch */ }
```

There is nothing magic inside — **two boxes collide if they overlap on all three axes at
once**. One axis where they do not overlap is enough for there to be no hit:

```c
overlapX = (a.min.x <= b.max.x) && (a.max.x >= b.min.x);
overlapY = (a.min.y <= b.max.y) && (a.max.y >= b.min.y);
overlapZ = (a.min.z <= b.max.z) && (a.max.z >= b.min.z);
hit      = overlapX && overlapY && overlapZ;
```

A debugging trick that saves hours: **draw the boxes**, with `DrawBoundingBox(box, GREEN)`.
Nine times out of ten the bug is that the box is not where you thought it was.

Others raylib ships that you will want soon: `CheckCollisionSpheres`,
`CheckCollisionBoxSphere`, and `GetRayCollisionBox` for shooting at / clicking on objects.

**Lesson:** `lesson05_collision.c` — move the blue cube; the orange box turns red when you
touch it and the bottom shows you, axis by axis, what is happening.

---

## 6. Making the wall actually stop you

Detecting the hit is half the job. The other half is **resolving** it, and the technique
used by almost every simple game is *move and check, one axis at a time*:

1. Work out how far you want to move this frame (`dx`, `dz`).
2. Try moving **along X only**. If you hit something, undo that X movement.
3. Try moving **along Z only**. If you hit something, undo that Z movement.

Separating the axes is what lets you **slide** along a wall instead of getting stuck. If
you checked both at once, brushing a wall diagonally would stop you dead.

```c
Vector3 attempt = camera.position;
attempt.x += dx;
if (HasCollision(attempt)) { /* no advance on X */ }
else camera.position.x = attempt.x;

attempt = camera.position;
attempt.z += dz;
if (HasCollision(attempt)) { /* no advance on Z */ }
else camera.position.z = attempt.z;
```

One note from the lesson: `camera.position` is the height of the **eyes** (1.7), so the
center of the player's body is half a body lower. Mixing up those two points is what makes
collisions feel "shifted upwards".

**Lesson:** `lesson06_scene.c` — a closed scene with walls and blocks; objects turn red
when touched and the walls stop you.

---

## 7. Putting it together: a silly little Doom

`lesson07_mini_doom.c` is the capstone: the level is an array of strings, collision is a
grid lookup instead of a box list, shooting is a `Ray` plus `GetRayCollisionBox()`, and the
crosshair, gun, health bar and minimap are plain 2D drawn after `EndMode3D()`.

It is walked through chapter by chapter in [`../manual/raylib-from-zero.pdf`](../manual/raylib-from-zero.pdf),
which is the long-form version of this file plus everything that comes before it.

---

## Cheat sheet

| I want to... | Function |
|---|---|
| start/end 3D | `BeginMode3D(cam)` / `EndMode3D()` |
| floor grid (XZ plane) | `DrawGrid(20, 1.0f)` |
| a cube | `DrawCube(center, widthX, heightY, lengthZ, color)` / `DrawCubeV(center, size, color)` |
| its edges | `DrawCubeWires(...)` / `DrawCubeWiresV(...)` |
| a 3D line | `DrawLine3D(a, b, color)` |
| a sphere | `DrawSphere(center, radius, color)` |
| an automatic camera | `UpdateCamera(&cam, CAMERA_FREE)` |
| capture/release the mouse | `DisableCursor()` / `EnableCursor()` |
| how far the mouse moved | `GetMouseDelta()` |
| seconds of the last frame | `GetFrameTime()` |
| collide two boxes | `CheckCollisionBoxes(a, b)` |
| see a collision box | `DrawBoundingBox(box, GREEN)` |
| turn a 3D point into screen space | `GetWorldToScreen(point, cam)` |

The vector math (`Vector3Add`, `Vector3Scale`, `Vector3Normalize`, `Vector3CrossProduct`,
`Vector3Distance`...) lives in `raymath.h`: it is a separate header, you have to include
it explicitly.

## Where to go next

- Loading a model: `LoadModel()` + `DrawModel()`.
- Lights and materials: raylib's `shaders/lighting` example.
- Clicking on 3D objects: `GetScreenToWorldRay()` + `GetRayCollisionBox()`.
- Gravity and jumping: keep a `velocityY`, subtract gravity from it every frame, and apply
  the same "move and check" technique to the Y axis too.
