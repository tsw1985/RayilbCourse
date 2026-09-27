/* =============================================================================
   CHEATSHEET MODULE 8/10 - rmodels: meshes, models, materials, 3D collisions
   -----------------------------------------------------------------------------
   The cheatsheet's rmodels module has five blocks:

     Basic 3D shapes     DrawCube, DrawSphere... immediate, no setup, no texture
     Mesh generation     GenMeshCube, GenMeshSphere... real geometry you own
     Model management    a Model = meshes + materials + a transform
     Model drawing       DrawModel, DrawModelEx, billboards
     Collision detection AABBs, spheres and ray casts

   The distinction that matters: DrawCube is a throwaway, it builds geometry
   every frame and cannot take a texture. A Model built from GenMeshCube lives
   on the GPU, can carry a material with textures, and can be transformed.

   Start with the immediate shapes. Move to Models the moment you want textures,
   lighting or thousands of instances.

   Controls: mouse drag orbits, wheel zooms, TAB switches what is drawn,
             LEFT CLICK on an object to pick it with a ray.
   ============================================================================= */

#include "raylib.h"
#include "raymath.h"

int main(void)
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1040, 700, "Module 8 - rmodels");
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position   = (Vector3){ 10.0f, 8.0f, 10.0f };
    camera.target     = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.up         = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy       = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // =========================================================================
    // MESH GENERATION -> MODEL
    // =========================================================================
    // A Mesh is raw geometry: vertices, normals, texture coordinates.
    // LoadModelFromMesh wraps it in a Model with a default white material.
    Mesh cubeMesh   = GenMeshCube(2.0f, 2.0f, 2.0f);
    Mesh sphereMesh = GenMeshSphere(1.2f, 24, 24);
    Mesh torusMesh  = GenMeshTorus(0.35f, 1.2f, 16, 32);

    Model cubeModel   = LoadModelFromMesh(cubeMesh);
    Model sphereModel = LoadModelFromMesh(sphereMesh);
    Model torusModel  = LoadModelFromMesh(torusMesh);

    // ---- A MATERIAL WITH A TEXTURE ------------------------------------------
    // This is the step DrawCube cannot do. We generate a checkerboard, upload
    // it, and plug it into the model's material as its diffuse map.
    Image checked = GenImageChecked(256, 256, 32, 32, (Color){ 70, 110, 180, 255 }, RAYWHITE);
    Texture2D tex = LoadTextureFromImage(checked);
    UnloadImage(checked);

    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = tex;

    // A billboard texture: always faces the camera
    Image glowImg = GenImageGradientRadial(128, 128, 0.2f, GOLD, BLANK);
    Texture2D glow = LoadTextureFromImage(glowImg);
    UnloadImage(glowImg);

    // =========================================================================
    // COLLISION TARGETS
    // =========================================================================
    Vector3 boxPos    = { -4.0f, 1.0f, 0.0f };
    Vector3 spherePos = {  0.0f, 1.2f, 0.0f };
    Vector3 torusPos  = {  4.0f, 1.0f, 0.0f };

    BoundingBox boxBounds = { (Vector3){ boxPos.x - 1, boxPos.y - 1, boxPos.z - 1 },
                              (Vector3){ boxPos.x + 1, boxPos.y + 1, boxPos.z + 1 } };

    int picked = -1;
    float pickDistance = 0.0f;
    int mode = 0;
    const char *modeNames[] = { "Models (GenMesh + material)", "Immediate shapes (DrawCube etc.)", "Wireframe" };

    while (!WindowShouldClose())
    {
        // Orbit with the right mouse button held, like a 3D editor
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        camera.position = Vector3Add(camera.position,
            Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)),
                         GetMouseWheelMove()*1.2f));

        if (IsKeyPressed(KEY_TAB)) mode = (mode + 1) % 3;

        // =====================================================================
        // RAY PICKING - turn a mouse click into a 3D ray and see what it hits
        // =====================================================================
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Ray ray = GetScreenToWorldRay(GetMousePosition(), camera);
            picked = -1;
            float best = 1e30f;

            RayCollision hitBox = GetRayCollisionBox(ray, boxBounds);
            if (hitBox.hit && hitBox.distance < best) { best = hitBox.distance; picked = 0; }

            RayCollision hitSphere = GetRayCollisionSphere(ray, spherePos, 1.2f);
            if (hitSphere.hit && hitSphere.distance < best) { best = hitSphere.distance; picked = 1; }

            // A torus has no primitive test: use its mesh's bounding box
            BoundingBox torusBounds = GetMeshBoundingBox(torusMesh);
            torusBounds.min = Vector3Add(torusBounds.min, torusPos);
            torusBounds.max = Vector3Add(torusBounds.max, torusPos);
            RayCollision hitTorus = GetRayCollisionBox(ray, torusBounds);
            if (hitTorus.hit && hitTorus.distance < best) { best = hitTorus.distance; picked = 2; }

            pickDistance = (picked >= 0) ? best : 0.0f;
        }

        float spin = (float)GetTime()*30.0f;

        BeginDrawing();
            ClearBackground((Color){ 22, 24, 30, 255 });

            BeginMode3D(camera);
                DrawGrid(20, 1.0f);

                if (mode == 0)
                {
                    // ---- MODEL DRAWING ---------------------------------------
                    DrawModel(cubeModel, boxPos, 1.0f, picked == 0 ? RED : WHITE);

                    // Ex = with a rotation axis and angle, and a per-axis scale
                    DrawModelEx(sphereModel, spherePos, (Vector3){ 0, 1, 0 }, spin,
                                (Vector3){ 1, 1, 1 }, picked == 1 ? RED : SKYBLUE);

                    DrawModelEx(torusModel, torusPos, (Vector3){ 1, 0.3f, 0 }, spin,
                                (Vector3){ 1, 1, 1 }, picked == 2 ? RED : GOLD);
                }
                else if (mode == 1)
                {
                    // ---- IMMEDIATE SHAPES: no models, no materials -----------
                    DrawCube(boxPos, 2, 2, 2, picked == 0 ? RED : MAROON);
                    DrawCubeWires(boxPos, 2, 2, 2, BLACK);

                    DrawSphere(spherePos, 1.2f, picked == 1 ? RED : SKYBLUE);
                    DrawSphereWires(spherePos, 1.2f, 12, 12, Fade(BLACK, 0.4f));

                    DrawCylinder(Vector3Subtract(torusPos, (Vector3){0,1,0}), 0.9f, 0.9f, 2.0f, 20,
                                 picked == 2 ? RED : GOLD);
                    DrawCylinderWires(Vector3Subtract(torusPos, (Vector3){0,1,0}), 0.9f, 0.9f, 2.0f, 20, Fade(BLACK, 0.4f));
                }
                else
                {
                    // ---- WIREFRAME: the same models, edges only -------------
                    DrawModelWires(cubeModel, boxPos, 1.0f, LIME);
                    DrawModelWires(sphereModel, spherePos, 1.0f, LIME);
                    DrawModelWires(torusModel, torusPos, 1.0f, LIME);
                }

                // ---- BILLBOARD: always faces the camera ----------------------
                DrawBillboard(camera, glow, (Vector3){ 0.0f, 4.0f, 0.0f }, 3.0f, WHITE);

                // ---- COLLISION VOLUMES, drawn so you can see them ------------
                DrawBoundingBox(boxBounds, picked == 0 ? RED : GREEN);
                DrawSphereWires(spherePos, 1.2f, 8, 8, Fade(picked == 1 ? RED : GREEN, 0.35f));
            EndMode3D();

            // ---- HUD ------------------------------------------------------
            DrawRectangle(0, 0, GetScreenWidth(), 92, Fade(BLACK, 0.75f));
            DrawText("rmodels", 16, 12, 24, RAYWHITE);
            DrawText(TextFormat("TAB: %s", modeNames[mode]), 16, 44, 17, GOLD);
            DrawText(picked >= 0
                     ? TextFormat("picked object %i at distance %.2f  (GetScreenToWorldRay + GetRayCollision*)", picked, pickDistance)
                     : "left click an object to pick it with a ray",
                     16, 68, 16, picked >= 0 ? LIME : GRAY);

            DrawRectangle(0, GetScreenHeight() - 48, GetScreenWidth(), 48, Fade(BLACK, 0.75f));
            DrawText("GenMeshCube/Sphere/Torus -> LoadModelFromMesh -> materials[0].maps[MATERIAL_MAP_DIFFUSE].texture",
                     12, GetScreenHeight() - 40, 15, RAYWHITE);
            DrawText("right drag orbit   wheel zoom   TAB mode   left click pick",
                     12, GetScreenHeight() - 20, 15, GRAY);
        EndDrawing();
    }

    // ---- UNLOAD -------------------------------------------------------------
    // UnloadModel frees the mesh it owns. The texture we plugged in is OURS,
    // so raylib will not free it: we must, and only once.
    UnloadModel(cubeModel);
    UnloadModel(sphereModel);
    UnloadModel(torusModel);
    UnloadTexture(tex);
    UnloadTexture(glow);

    CloseWindow();
    return 0;
}
