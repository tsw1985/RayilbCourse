/* =============================================================================
   LESSON 07 - A silly little Doom
   -----------------------------------------------------------------------------
   The capstone. Everything from lessons 01-06 plus the four ideas that turn a
   walkable scene into a game:

     1. A MAP AS TEXT. The level is an array of strings, '#' is a wall and '.'
        is floor. Designing a level becomes typing characters. This is literally
        how many classic games stored their levels.

     2. GRID COLLISION. Instead of testing against a list of boxes, we convert
        the player's world position into a cell index and ask "is that cell a
        wall?". It is O(1) no matter how big the map is, and it is how Wolf3D
        and Doom did it. We still resolve one axis at a time so you can slide.

     3. SHOOTING WITH A RAY. A Ray is an origin plus a direction. We fire one
        straight out of the camera and ask GetRayCollisionBox() what it meets.
        The nearest thing hit wins; if a wall is nearer than the enemy, you
        missed.

     4. A HUD. Drawn AFTER EndMode3D, so it is plain 2D pixels: crosshair, gun,
        health bar and a minimap. This is where the 2D course pays off.

   Controls: WASD move, mouse look, SHIFT run, LEFT CLICK shoot, R restart,
             TAB release the mouse, ESC quit.
   ============================================================================= */

#include "raylib.h"
#include "raymath.h"
#include <math.h>

#define MAP_ROWS      15
#define MAP_COLS      15
#define CELL          2.0f      // world units per map cell
#define WALL_HEIGHT   3.0f
#define EYE_HEIGHT    1.7f
#define PLAYER_RADIUS 0.3f
#define MAX_ENEMIES   8

// '#' wall   '.' floor   'S' player start   'E' enemy start
static const char *MAP[MAP_ROWS] = {
    "###############",
    "#S....#.......#",
    "#.....#...E...#",
    "#.###.#.#####.#",
    "#.#...#.....#.#",
    "#.#.#######.#.#",
    "#...#..E..#...#",
    "###.#.###.#.###",
    "#...#.#.#.#...#",
    "#.###.#.#.###.#",
    "#....E#.#.....#",
    "#####.#.#.#####",
    "#.....#.#....E#",
    "#..E..........#",
    "###############",
};

typedef struct Enemy {
    Vector3 position;
    bool    alive;
    float   hitFlash;     // seconds left of the "I was shot" flash
} Enemy;

// ---------------------------------------------------------------------------
// MAP HELPERS
// ---------------------------------------------------------------------------

// The centre of cell (col,row) in world coordinates.
// Note how the map's COLUMN becomes X and the map's ROW becomes Z. Y is height,
// so a top-down map only has two of the three axes in it.
static Vector3 CellToWorld(int col, int row, float y)
{
    return (Vector3){ col*CELL, y, row*CELL };
}

// Is the world point (x, z) inside a wall cell? Anything off the map counts as
// a wall, which saves us from ever indexing out of bounds.
static bool IsWall(float x, float z)
{
    int col = (int)floorf(x/CELL + 0.5f);
    int row = (int)floorf(z/CELL + 0.5f);

    if (col < 0 || col >= MAP_COLS || row < 0 || row >= MAP_ROWS) return true;

    return (MAP[row][col] == '#');
}

// Can a circle of PLAYER_RADIUS stand at (x, z)? We test the four corners of
// the square that contains it: cheap, and good enough for square maps.
static bool CanStandAt(float x, float z)
{
    const float r = PLAYER_RADIUS;
    return !IsWall(x - r, z - r) && !IsWall(x + r, z - r)
        && !IsWall(x - r, z + r) && !IsWall(x + r, z + r);
}

static BoundingBox EnemyBox(Vector3 position)
{
    return (BoundingBox){
        (Vector3){ position.x - 0.4f, position.y - 0.9f, position.z - 0.4f },
        (Vector3){ position.x + 0.4f, position.y + 0.9f, position.z + 0.4f }
    };
}

// ---------------------------------------------------------------------------
// GAME STATE
// ---------------------------------------------------------------------------

typedef struct Game {
    Camera3D camera;
    float    yaw, pitch;
    float    health;
    int      ammo;
    int      kills;
    float    bobTimer;       // drives the walking head bob
    float    muzzleFlash;    // seconds left of the gun flash
    Enemy    enemies[MAX_ENEMIES];
    int      enemyCount;
} Game;

static void ResetGame(Game *g)
{
    g->camera = (Camera3D){ 0 };
    g->camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    g->camera.fovy       = 75.0f;              // a wide, Doom-ish lens
    g->camera.projection = CAMERA_PERSPECTIVE;

    g->yaw = 0.0f; g->pitch = 0.0f;
    g->health = 100.0f;
    g->ammo = 40;
    g->kills = 0;
    g->bobTimer = 0.0f;
    g->muzzleFlash = 0.0f;
    g->enemyCount = 0;

    // Read the map once and place the player and the enemies
    for (int row = 0; row < MAP_ROWS; row++)
    {
        for (int col = 0; col < MAP_COLS; col++)
        {
            if (MAP[row][col] == 'S')
            {
                g->camera.position = CellToWorld(col, row, EYE_HEIGHT);
            }
            else if (MAP[row][col] == 'E' && g->enemyCount < MAX_ENEMIES)
            {
                g->enemies[g->enemyCount].position = CellToWorld(col, row, 0.9f);
                g->enemies[g->enemyCount].alive = true;
                g->enemies[g->enemyCount].hitFlash = 0.0f;
                g->enemyCount++;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// MAIN
// ---------------------------------------------------------------------------

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1024, 640, "Lesson 07 - A silly little Doom");
    SetTargetFPS(60);

    Game game;
    ResetGame(&game);

    DisableCursor();

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        bool alive = (game.health > 0.0f);

        if (IsKeyPressed(KEY_TAB)) { if (IsCursorHidden()) EnableCursor(); else DisableCursor(); }
        if (IsKeyPressed(KEY_R)) { ResetGame(&game); DisableCursor(); }

        // =====================================================================
        // LOOK  (lesson 04)
        // =====================================================================
        if (IsCursorHidden() && alive)
        {
            Vector2 mouse = GetMouseDelta();
            game.yaw   += mouse.x*0.003f;
            game.pitch -= mouse.y*0.003f;
            if (game.pitch >  1.0f) game.pitch =  1.0f;   // Doom barely let you look up
            if (game.pitch < -1.0f) game.pitch = -1.0f;
        }

        Vector3 forward = { cosf(game.pitch)*cosf(game.yaw),
                            sinf(game.pitch),
                            cosf(game.pitch)*sinf(game.yaw) };
        Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, (Vector3){ 0, 1, 0 }));
        Vector3 flatForward = Vector3Normalize((Vector3){ forward.x, 0.0f, forward.z });

        // =====================================================================
        // MOVE  (lesson 06, but against the grid instead of a box list)
        // =====================================================================
        Vector3 dir = { 0 };
        if (alive)
        {
            if (IsKeyDown(KEY_W)) dir = Vector3Add(dir, flatForward);
            if (IsKeyDown(KEY_S)) dir = Vector3Subtract(dir, flatForward);
            if (IsKeyDown(KEY_D)) dir = Vector3Add(dir, right);
            if (IsKeyDown(KEY_A)) dir = Vector3Subtract(dir, right);
        }

        bool walking = (Vector3Length(dir) > 0.0f);
        if (walking) dir = Vector3Normalize(dir);

        float speed = IsKeyDown(KEY_LEFT_SHIFT) ? 7.0f : 4.0f;

        // One axis at a time, so a wall slides you along instead of stopping you
        float nextX = game.camera.position.x + dir.x*speed*dt;
        if (CanStandAt(nextX, game.camera.position.z)) game.camera.position.x = nextX;

        float nextZ = game.camera.position.z + dir.z*speed*dt;
        if (CanStandAt(game.camera.position.x, nextZ)) game.camera.position.z = nextZ;

        // Head bob: a sine wave on the eye height while you walk. Pure garnish,
        // but it is most of what makes movement "feel" right.
        if (walking) game.bobTimer += dt*speed*1.6f;
        float bob = walking ? sinf(game.bobTimer)*0.045f : 0.0f;
        game.camera.position.y = EYE_HEIGHT + bob;

        game.camera.target = Vector3Add(game.camera.position, forward);

        // =====================================================================
        // SHOOT: one ray, straight out of the eye
        // =====================================================================
        if (game.muzzleFlash > 0.0f) game.muzzleFlash -= dt;

        if (alive && IsCursorHidden() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && game.ammo > 0)
        {
            game.ammo--;
            game.muzzleFlash = 0.06f;

            Ray shot = { game.camera.position, forward };

            // How far away is the wall in front of us? Step along the ray until
            // we are inside a wall cell. Crude, but exactly how a raycaster
            // works, and it stops you shooting through walls.
            float wallDistance = 100.0f;
            for (float t = 0.1f; t < 60.0f; t += 0.1f)
            {
                Vector3 p = Vector3Add(shot.position, Vector3Scale(shot.direction, t));
                if (IsWall(p.x, p.z)) { wallDistance = t; break; }
            }

            // Nearest enemy the ray touches, if it is closer than that wall
            int   target = -1;
            float best = wallDistance;

            for (int i = 0; i < game.enemyCount; i++)
            {
                if (!game.enemies[i].alive) continue;

                RayCollision hit = GetRayCollisionBox(shot, EnemyBox(game.enemies[i].position));
                if (hit.hit && hit.distance < best) { best = hit.distance; target = i; }
            }

            if (target >= 0)
            {
                game.enemies[target].alive = false;
                game.enemies[target].hitFlash = 0.25f;
                game.kills++;
            }
        }

        // =====================================================================
        // ENEMIES: walk towards you, hurt you when they reach you
        // =====================================================================
        for (int i = 0; i < game.enemyCount; i++)
        {
            Enemy *e = &game.enemies[i];
            if (e->hitFlash > 0.0f) e->hitFlash -= dt;
            if (!e->alive || !alive) continue;

            Vector3 toPlayer = Vector3Subtract(game.camera.position, e->position);
            toPlayer.y = 0.0f;
            float distance = Vector3Length(toPlayer);

            if (distance < 12.0f && distance > 0.9f)
            {
                Vector3 step = Vector3Scale(Vector3Normalize(toPlayer), 1.6f*dt);

                // Same per-axis trick, so they do not walk into walls either
                if (CanStandAt(e->position.x + step.x, e->position.z)) e->position.x += step.x;
                if (CanStandAt(e->position.x, e->position.z + step.z)) e->position.z += step.z;
            }
            else if (distance <= 0.9f)
            {
                game.health -= 25.0f*dt;      // 4 seconds of contact and you are done
                if (game.health < 0.0f) game.health = 0.0f;
            }
        }

        // =====================================================================
        // DRAW
        // =====================================================================
        BeginDrawing();
            ClearBackground((Color){ 12, 12, 16, 255 });

            // ---------------- 3D WORLD ----------------
            BeginMode3D(game.camera);

                // Floor and ceiling: two big flat planes
                DrawPlane((Vector3){ MAP_COLS*CELL/2, 0.0f, MAP_ROWS*CELL/2 },
                          (Vector2){ MAP_COLS*CELL, MAP_ROWS*CELL }, (Color){ 48, 42, 38, 255 });
                DrawPlane((Vector3){ MAP_COLS*CELL/2, WALL_HEIGHT, MAP_ROWS*CELL/2 },
                          (Vector2){ MAP_COLS*CELL, MAP_ROWS*CELL }, (Color){ 26, 26, 32, 255 });

                // The walls: one cube per '#' cell. Colour varies a little per
                // cell so the corridors do not look like one flat mass.
                for (int row = 0; row < MAP_ROWS; row++)
                {
                    for (int col = 0; col < MAP_COLS; col++)
                    {
                        if (MAP[row][col] != '#') continue;

                        Vector3 centre = CellToWorld(col, row, WALL_HEIGHT/2);
                        int shade = 88 + ((col*7 + row*13) % 26);
                        Color wall = (Color){ (unsigned char)shade, (unsigned char)(shade - 24), (unsigned char)(shade - 34), 255 };

                        DrawCube(centre, CELL, WALL_HEIGHT, CELL, wall);
                        DrawCubeWires(centre, CELL, WALL_HEIGHT, CELL, (Color){ 30, 26, 24, 255 });
                    }
                }

                // The enemies: a body and a head, nothing fancy
                for (int i = 0; i < game.enemyCount; i++)
                {
                    Enemy *e = &game.enemies[i];
                    if (!e->alive) continue;

                    Color body = (e->hitFlash > 0.0f) ? WHITE : (Color){ 150, 40, 40, 255 };

                    DrawCylinder(Vector3Subtract(e->position, (Vector3){ 0, 0.9f, 0 }),
                                 0.35f, 0.45f, 1.4f, 10, body);
                    DrawSphere(Vector3Add(e->position, (Vector3){ 0, 0.55f, 0 }), 0.28f, body);

                    // Two eyes that always face you: the cheapest way to make a
                    // shape feel like it is looking at the player.
                    Vector3 toPlayer = Vector3Normalize(Vector3Subtract(game.camera.position, e->position));
                    Vector3 eyeSide = Vector3Normalize(Vector3CrossProduct(toPlayer, (Vector3){ 0, 1, 0 }));
                    Vector3 eyeBase = Vector3Add(e->position, (Vector3){ 0, 0.6f, 0 });
                    eyeBase = Vector3Add(eyeBase, Vector3Scale(toPlayer, 0.22f));
                    DrawSphere(Vector3Add(eyeBase, Vector3Scale(eyeSide,  0.10f)), 0.055f, YELLOW);
                    DrawSphere(Vector3Add(eyeBase, Vector3Scale(eyeSide, -0.10f)), 0.055f, YELLOW);
                }

            EndMode3D();
            // ---------------- 2D HUD ----------------
            // From here on, plain screen pixels. Nothing below moves with the camera.

            int sw = GetScreenWidth();
            int sh = GetScreenHeight();

            // Crosshair
            DrawLine(sw/2 - 12, sh/2, sw/2 - 4, sh/2, RAYWHITE);
            DrawLine(sw/2 + 4, sh/2, sw/2 + 12, sh/2, RAYWHITE);
            DrawLine(sw/2, sh/2 - 12, sw/2, sh/2 - 4, RAYWHITE);
            DrawLine(sw/2, sh/2 + 4, sw/2, sh/2 + 12, RAYWHITE);

            // The gun: a couple of rectangles at the bottom of the screen that
            // bob with your steps. That is all a "weapon sprite" has to be.
            {
                float gunBob = sinf(game.bobTimer)*6.0f;
                int gx = sw/2 + 90;
                int gy = sh - 100 + (int)gunBob;

                DrawRectangle(gx - 18, gy, 36, 110, (Color){ 60, 60, 68, 255 });
                DrawRectangle(gx - 10, gy - 70, 20, 80, (Color){ 40, 40, 46, 255 });
                DrawRectangle(gx - 6, gy - 74, 12, 10, (Color){ 90, 90, 98, 255 });

                if (game.muzzleFlash > 0.0f)
                {
                    DrawCircle(gx, gy - 78, 26, Fade(YELLOW, 0.9f));
                    DrawCircle(gx, gy - 78, 14, Fade(WHITE, 0.9f));
                }
            }

            // Health and ammo
            DrawRectangle(20, sh - 60, 240, 40, Fade(BLACK, 0.6f));
            DrawRectangle(28, sh - 34, (int)(224.0f*game.health/100.0f), 8,
                          game.health > 30.0f ? (Color){ 90, 200, 90, 255 } : RED);
            DrawText(TextFormat("HEALTH %3.0f", game.health), 28, sh - 54, 18, RAYWHITE);

            DrawRectangle(sw - 200, sh - 60, 180, 40, Fade(BLACK, 0.6f));
            DrawText(TextFormat("AMMO %2i", game.ammo), sw - 190, sh - 54, 18, game.ammo > 0 ? RAYWHITE : RED);
            DrawText(TextFormat("KILLS %i/%i", game.kills, game.enemyCount), sw - 190, sh - 34, 16, GRAY);

            // Minimap: the same map, drawn as flat squares. Map column -> screen
            // X, map row -> screen Y. This is why a top-down map only needs two
            // of the three axes.
            {
                const int tile = 9;
                const int ox = 20, oy = 20;

                DrawRectangle(ox - 6, oy - 6, MAP_COLS*tile + 12, MAP_ROWS*tile + 12, Fade(BLACK, 0.6f));

                for (int row = 0; row < MAP_ROWS; row++)
                    for (int col = 0; col < MAP_COLS; col++)
                        if (MAP[row][col] == '#')
                            DrawRectangle(ox + col*tile, oy + row*tile, tile - 1, tile - 1, (Color){ 120, 100, 90, 255 });

                for (int i = 0; i < game.enemyCount; i++)
                {
                    if (!game.enemies[i].alive) continue;
                    DrawCircle(ox + (int)(game.enemies[i].position.x/CELL*tile),
                               oy + (int)(game.enemies[i].position.z/CELL*tile), 3, RED);
                }

                int px = ox + (int)(game.camera.position.x/CELL*tile);
                int py = oy + (int)(game.camera.position.z/CELL*tile);
                DrawCircle(px, py, 3, SKYBLUE);

                // The little line showing where you are facing: on the minimap,
                // world X is the screen X and world Z is the screen Y.
                DrawLine(px, py, px + (int)(forward.x*14), py + (int)(forward.z*14), SKYBLUE);
            }

            if (!alive)
            {
                DrawRectangle(0, 0, sw, sh, Fade(MAROON, 0.35f));
                const char *txt = "YOU DIED";
                DrawText(txt, sw/2 - MeasureText(txt, 60)/2, sh/2 - 60, 60, RAYWHITE);
                DrawText("press R to restart", sw/2 - MeasureText("press R to restart", 20)/2, sh/2 + 10, 20, RAYWHITE);
            }
            else if (game.kills == game.enemyCount)
            {
                const char *txt = "LEVEL CLEAR";
                DrawText(txt, sw/2 - MeasureText(txt, 40)/2, 40, 40, GOLD);
            }

            DrawFPS(sw - 90, 20);
        EndDrawing();
    }

    EnableCursor();
    CloseWindow();
    return 0;
}
