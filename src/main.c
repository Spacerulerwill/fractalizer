#include "raylib.h"
#include "raymath.h"
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <complex.h>

typedef enum {
    RENDER_MODE_FRACTAL,
    RENDER_MODE_JULIA_SET,
    RENDER_MODE_FRACTAL_WITH_JULIA_SET_OVERLAY
} RenderMode;

typedef enum {
    FRACTAL_TYPE_MANDELBROT,
    FRACTAL_TYPE_BURNING_SHIP,
    FRACTAL_TYPE_TRICORN
} FractalType;

static void show_complex_path(float complex z, float complex c, int iterations, float aspectRatio, float screenWidth, float screenHeight, float zoom, Vector2 fractalArgandCenter) {
    Vector2 previousScreenPos = { 0 };

    for (int i = 0; i < iterations; i++) {
        z = (z * z) + c;

        float screenX = creal(z) - fractalArgandCenter.x;
        float screenY = cimag(z) - fractalArgandCenter.y;
        screenX /= aspectRatio * zoom;
        screenY /= -1.0f * zoom;
        screenX = screenWidth / 2.0f + screenX * screenWidth / 2.0f;
        screenY = screenHeight / 2.0f - screenY * screenHeight / 2.0f;

        DrawCircle(screenX, screenY, 2.0f, BLUE);

        if (i > 0) {
            DrawLineEx(previousScreenPos, (Vector2){ screenX, screenY }, 1.0f, RED);
        }

        previousScreenPos = (Vector2){ screenX, screenY };
    }
}

int main(void)
{
    int screenWidth = 1280;
    int screenHeight = 720;
    float aspectRatio = (float)screenWidth / (float)screenHeight;

    // Raylib init
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Fractalizer");
    SetTargetFPS(144);

    // Shader loading plus shader uniform locations
    Shader shader = LoadShader(0, "shader.fs");
    int resolutionLoc = GetShaderLocation(shader, "resolution");
    int fractalCenterArgandLoc = GetShaderLocation(shader, "fractalCenterArgand");
    int fractalZoomLoc = GetShaderLocation(shader, "fractalZoom");
    int juliaSetCenterArgandLoc = GetShaderLocation(shader, "juliaSetCenterArgand");
    int juliaSetZoomLoc = GetShaderLocation(shader, "juliaSetZoom");
    int juliaSetMousePosArgandLoc = GetShaderLocation(shader, "juliaSetMousePosArgand");
    int fractalTypeLoc = GetShaderLocation(shader, "fractalType");
    int renderModeLoc = GetShaderLocation(shader, "renderMode");
    int iterationsLoc = GetShaderLocation(shader, "iterations");

    // Fractal state
    Vector2 fractalCenterArgand = { 0.0f, 0.0f };       
    float fractalZoom = 2.0f;
    Vector2 juliaSetCenterArgand = { 0.0f, 0.0f };
    float juliaSetZoom = 2.0f;
    Vector2 juliaSetMousePosArgand = { 0.0f, 0.0f };
    FractalType fractalType = FRACTAL_TYPE_MANDELBROT;
    RenderMode renderMode = RENDER_MODE_FRACTAL;
    int iterations = 200;
    bool isJuliaSetOn = false; // Only for the checkbox, julia set mode is handled by rendermode
    bool isJuliaSetFrozen = false;
    bool isComplexPathShown = false;

    // Nuklear init
    int fontSize = 10;
    struct nk_context* ctx = InitNuklear(fontSize);

    // Main loop
    while (!WindowShouldClose())
    {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        aspectRatio = (float)screenWidth / (float)screenHeight;
        Vector2 mouseScreenPos = GetMousePosition();

        // Calculate position of mouse cursor on argand diagram
        float normX = ((mouseScreenPos.x / (float)screenWidth) * 2.0f - 1.0f) * aspectRatio * fractalZoom;
        float normY = ((mouseScreenPos.y / (float)screenHeight) * 2.0f - 1.0f) * fractalZoom;
        Vector2 mousePosArgand = { fractalCenterArgand.x + normX * 0.5f, fractalCenterArgand.y - normY * 0.5f };

        BeginDrawing();
        // Final fractal rendering
        ClearBackground(RAYWHITE);
        // Update all our shader uniforms and draw fractal
        BeginShaderMode(shader);
        SetShaderValue(shader, resolutionLoc, (int[2]) { screenWidth, screenHeight }, SHADER_UNIFORM_IVEC2);
        SetShaderValue(shader, fractalCenterArgandLoc, &fractalCenterArgand, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, fractalZoomLoc, &fractalZoom, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, juliaSetCenterArgandLoc, &juliaSetCenterArgand, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, juliaSetZoomLoc, &juliaSetZoom, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, juliaSetMousePosArgandLoc, &juliaSetMousePosArgand, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, fractalTypeLoc, &fractalType, SHADER_UNIFORM_INT);
        SetShaderValue(shader, renderModeLoc, &renderMode, SHADER_UNIFORM_INT);
        SetShaderValue(shader, iterationsLoc, &iterations, SHADER_UNIFORM_INT);
        DrawRectangleRec((Rectangle) { 0, 0, (float)screenWidth, (float)screenHeight }, WHITE);
        EndShaderMode();

        // GUI
        UpdateNuklear(ctx);
        if (nk_begin(ctx, "Control Panel", nk_rect(10, 10, 220, 220),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_property_int(ctx, "Iterations", 10, &iterations, 1000, 10, 1.0f);
            char mouseAtText[128];
            if (mousePosArgand.y < 0) {
                snprintf(mouseAtText, sizeof(mouseAtText), "Mouse at: %f - %fi", mousePosArgand.x, mousePosArgand.y * -1.0f);
            } else {
                snprintf(mouseAtText, sizeof(mouseAtText), "Mouse at: %f + %fi", mousePosArgand.x, mousePosArgand.y);
            }
            nk_label(ctx, mouseAtText, NK_TEXT_LEFT);
            if (nk_checkbox_label(ctx, "Julia Set", &isJuliaSetOn)) {
                renderMode = isJuliaSetOn ? RENDER_MODE_JULIA_SET : RENDER_MODE_FRACTAL;
            }
            
            if (renderMode == RENDER_MODE_FRACTAL) {
                nk_checkbox_label(ctx, "Trace Complex Number Path", &isComplexPathShown);
                nk_label(
                    ctx,
                    "Controls:\n"
                    "J - Toggle julia set",
                    NK_LEFT
                );
            } else {
                nk_label(
                    ctx,
                    "Controls:\n"
                    "J - Toggle julia set\n"
                    "TAB - Show fractal overlay",
                    NK_LEFT
                );
            }
        }

        nk_end(ctx);
        DrawNuklear(ctx);

        // Conditional stuff
        if (renderMode == RENDER_MODE_FRACTAL) {
            // switching to julia set mode
            if (IsKeyPressed(KEY_J)) 
                renderMode = RENDER_MODE_JULIA_SET;
            // Tracing complex path
            if (isComplexPathShown) {
                show_complex_path(0, mousePosArgand.x + mousePosArgand.y * I, 25, aspectRatio, screenWidth, screenHeight, fractalZoom, fractalCenterArgand);
            }
        } else {
            // Switching back to fractal mode
            if (IsKeyPressed(KEY_J)) 
                renderMode = RENDER_MODE_FRACTAL;
            // Switching to julia set with fractal set overlay
            if (IsKeyPressed(KEY_TAB))
                renderMode = RENDER_MODE_FRACTAL_WITH_JULIA_SET_OVERLAY;
            else if (IsKeyReleased(KEY_TAB))
                renderMode = RENDER_MODE_JULIA_SET;
            // Freezing julia set
            if (IsKeyPressed(KEY_F))
                isJuliaSetFrozen = !isJuliaSetFrozen;
            // Updating mouse position if julia set is not frozen
            if (!isJuliaSetFrozen)
                juliaSetMousePosArgand = mousePosArgand;
        }
        EndDrawing();
    }

    // Cleanup
    UnloadNuklear(ctx);
    UnloadShader(shader);
    CloseWindow();
    return EXIT_SUCCESS;
}
