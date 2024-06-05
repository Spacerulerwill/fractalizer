#include "raylib.h"
#include "raymath.h"
#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <complex.h>

static const float zoomSpeed = 1.1f;

static void handle_dragging(struct nk_context* ctx, Vector2* center, bool* isDragging, Vector2* dragStart, Vector2 mouseScreenPos, Vector2* offset, float screenWidth, float screenHeight, float aspectRatio, float zoom) {
    if (!nk_window_is_any_hovered(ctx)) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            *isDragging = true;
            *dragStart = mouseScreenPos;
        }
    }
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (*isDragging) {
            *offset = Vector2Subtract(*dragStart, GetMousePosition());
            offset->x /= screenWidth / (aspectRatio * zoom);
            offset->y /= screenHeight * -1.0f / zoom;
        }
    }
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (*isDragging) {
            *isDragging = false;
            *center = Vector2Add(*center, *offset);
            *offset = (Vector2){ 0.0f, 0.0f };
        }
    }
}

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
    Shader shader = LoadShader(0, "shader.fs");
    int locationLoc = GetShaderLocation(shader, "location");
    int mousePocLoc = GetShaderLocation(shader, "mousePos");
    int zoomLoc = GetShaderLocation(shader, "zoom");
    int resolutionLoc = GetShaderLocation(shader, "resolution");
    int iterationsLoc = GetShaderLocation(shader, "iterations");
    int juliaLoc = GetShaderLocation(shader, "isJuliaModeEnabled");
    SetTargetFPS(144);

    // Nuklear init
    int fontSize = 10;
    struct nk_context* ctx = InitNuklear(fontSize);

    // Dragging
    Vector2 fractalCenterArgand = { 0.0f, 0.0f }; // Where fractal is currently centered on argand diagram
    Vector2 juliaSetCenterArgand = { 0.0f, 0.0f };
    Vector2 mouseLocationArgand = { 0.0f, 0.0f }; // Where the mouse is pointing on the argand diagram
    Vector2 dragStart = { 0.0f, 0.0f }; // Where a drag started
    Vector2 offset = { 0.0f, 0.0f }; // The current offset from the drag
    bool isDragging = false;
    // Fractal properties
    float fractalZoom = 2.0f;
    float juliaZoom = 2.0f;
    int iterations = 200;
    bool isComplexPathShown = false;
    bool isJuliaModeEnabled = false;
    bool isJuliaSetFrozen = false;

    // Main loop
    while (!WindowShouldClose())
    {
        screenWidth = GetScreenWidth();
        screenHeight = GetScreenHeight();
        aspectRatio = (float)screenWidth / (float)screenHeight;
        Vector2 mouseScreenPos = GetMousePosition();

        // Keybinds
        // Toggle julia set mode
        if (IsKeyPressed(KEY_J)) {
            isDragging = false;
            if (!isJuliaModeEnabled) {
                fractalCenterArgand = Vector2Add(fractalCenterArgand, offset);
                offset = (Vector2){ 0.0f, 0.0f }; // Reset offset
            }
            else {
                juliaSetCenterArgand = Vector2Add(juliaSetCenterArgand, offset);
                offset = (Vector2){ 0.0f, 0.0f }; // Reset offset
            }
            isJuliaModeEnabled = !isJuliaModeEnabled;
        }
        // Freeze julia set
        if (isJuliaModeEnabled && IsKeyPressed(KEY_F)) {
            isJuliaSetFrozen = !isJuliaSetFrozen;
        }
        if (IsKeyPressed(KEY_R)) {
            isDragging = false;
            if (isJuliaModeEnabled) {
                juliaSetCenterArgand = (Vector2){ 0.0f, 0.0f };
                juliaZoom = 2.0f;
            }
            else {
                fractalCenterArgand = (Vector2){ 0.0f, 0.0f };
                fractalZoom = 2.0f;
            }
            offset = (Vector2){ 0.0f, 0.0f };
        }

        // Dragging
        if (!isJuliaModeEnabled) {
            handle_dragging(ctx, &fractalCenterArgand, &isDragging, &dragStart, mouseScreenPos, &offset, (float)screenWidth, (float)screenHeight, aspectRatio, fractalZoom);
        }
        else if (isJuliaSetFrozen) {
            handle_dragging(ctx, &juliaSetCenterArgand, &isDragging, &dragStart, mouseScreenPos, &offset, (float)screenWidth, (float)screenHeight, aspectRatio, juliaZoom);
        }

        // Fractal location
        Vector2 finalLocation = Vector2Add(isJuliaModeEnabled ? juliaSetCenterArgand : fractalCenterArgand, offset);

        // Mouse argand location
        if (!isJuliaSetFrozen) {
            float normX = (mouseScreenPos.x / (float)screenWidth) * 2.0f - 1.0f;
            float normY = (mouseScreenPos.y / (float)screenHeight) * 2.0f - 1.0f;
            normX *= aspectRatio * (isJuliaModeEnabled ? juliaZoom : fractalZoom);
            normY *= (isJuliaModeEnabled ? juliaZoom : fractalZoom);
            mouseLocationArgand.x = fractalCenterArgand.x + normX * 0.5f;
            mouseLocationArgand.y = fractalCenterArgand.y - normY * 0.5f;
        }

        // Zoom with mouse wheel
        float scrollDelta = GetMouseWheelMove();
        if (scrollDelta != 0.0f) {
            if (isJuliaModeEnabled) {
                if (isJuliaSetFrozen) {
                    juliaZoom *= scrollDelta > 0 ? 1.0f / zoomSpeed : zoomSpeed;
                }
            }
            else {
                fractalZoom *= scrollDelta > 0 ? 1.0f / zoomSpeed : zoomSpeed;
            }
        }

        // GUI
        UpdateNuklear(ctx);
        if (nk_begin(ctx, "Control Panel", nk_rect(10, 10, 220, 220),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(ctx, 20, 1);
            nk_property_int(ctx, "Iterations", 10, &iterations, 1000, 10, 1.0f);
            nk_checkbox_label(ctx, "Julia Set", &isJuliaModeEnabled);
            if (!isJuliaModeEnabled)
                nk_checkbox_label(ctx, "Trace Complex Number Path", &isComplexPathShown);
            char centeredAtText[128];
            if (finalLocation.y < 0) {
                snprintf(centeredAtText, sizeof(centeredAtText), "Centered at: %f - %fi", finalLocation.x, finalLocation.y * -1.0f);
            } else {
                snprintf(centeredAtText, sizeof(centeredAtText), "Centered at: %f + %fi", finalLocation.x, finalLocation.y);
            }
            char mouseAtText[128];
            if (mouseLocationArgand.y < 0) {
                snprintf(mouseAtText, sizeof(centeredAtText), "Mouse at: %f - %fi", mouseLocationArgand.x, mouseLocationArgand.y * -1.0f);
            } else {
                snprintf(mouseAtText, sizeof(centeredAtText), "Mouse at: %f + %fi", mouseLocationArgand.x, mouseLocationArgand.y);
            }
            nk_label(ctx, centeredAtText, NK_TEXT_LEFT);
            nk_label(ctx, mouseAtText, NK_TEXT_LEFT);
        }
        nk_end(ctx);

        // Draw fractal
        BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginShaderMode(shader);
        SetShaderValue(shader, locationLoc, &finalLocation, SHADER_UNIFORM_VEC2);
        SetShaderValue(shader, resolutionLoc, (int[2]) { screenWidth, screenHeight }, SHADER_UNIFORM_IVEC2);
        SetShaderValue(shader, zoomLoc, isJuliaModeEnabled ? &juliaZoom : &fractalZoom, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, iterationsLoc, &iterations, SHADER_UNIFORM_INT);
        SetShaderValue(shader, mousePocLoc, &mouseLocationArgand, SHADER_UNIFORM_VEC2);
        int temp = isJuliaModeEnabled ? 1 : 0;
        SetShaderValue(shader, juliaLoc, &temp, SHADER_UNIFORM_INT);
        DrawRectangleRec((Rectangle) { 0, 0, (float)screenWidth, (float)screenHeight }, WHITE);
        EndShaderMode();
        
        // Draw complex paths
        if (isComplexPathShown && !isJuliaModeEnabled){
            show_complex_path(0, mouseLocationArgand.x + mouseLocationArgand.y * I, 25, aspectRatio, screenWidth, screenHeight, fractalZoom, fractalCenterArgand);
        }
        DrawNuklear(ctx);
        EndDrawing();
    }

    // Cleanup
    UnloadNuklear(ctx);
    UnloadShader(shader);
    CloseWindow();
    return EXIT_SUCCESS;
}
