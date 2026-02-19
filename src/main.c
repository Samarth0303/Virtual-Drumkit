#include "raylib.h"
#include <stdio.h>

// Game states
typedef enum {
    STATE_MENU,
    STATE_SELECT_KIT,
    STATE_PLAYING,
    STATE_PAUSED
} GameState;

// Structure to define a Drum Kit category
typedef struct {
    const char *kitName;
    const char *samples[4]; // Paths to 4 wav files
} KitCategory;

// Structure for the interactive pads
typedef struct {
    Rectangle rect;
    Color baseColor;
    Color hitColor;
    KeyboardKey key;
    Sound sample;
    float flashTime;
    float flashRemaining;
    const char *label;
} Pad;

// Helper: trigger pad hit (sound + flash)
void HitPad(Pad *p) {
    if (p->sample.frameCount > 0) { // Ensure sound is loaded
        PlaySound(p->sample);
        p->flashRemaining = p->flashTime;
    }
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Beat Box - Pro Edition");
    SetTargetFPS(60);
    InitAudioDevice();

    GameState currentState = STATE_MENU;
    int selectedKitIndex = 0;

    // 1. Define the 4 Sections with 4 audio files each
    KitCategory kits[4] = {
        { "ACOUSTIC",   { "assets/kick.wav", "assets/snare.wav", "assets/hihat.wav", "assets/clap.wav" } },
        { "ELECTRONIC", { "assets/elec_kick.wav", "assets/elec_snare.wav", "assets/elec_hihat.wav", "assets/elec_clap.wav" } },
        { "PERCUSSION", { "assets/bongo.wav", "assets/conga.wav", "assets/shaker.wav", "assets/rim.wav" } },
        { "INDUSTRIAL", { "assets/metal.wav", "assets/steam.wav", "assets/clank.wav", "assets/zap.wav" } }
    };

    // Placeholder sounds to be loaded dynamically
    Sound padSounds[4] = { 0 };

    // Layout configuration for pads
    const float margin = 30.0f;
    const float padW = (screenWidth - margin * 3) / 2;
    const float padH = (screenHeight - margin * 3) / 2;

    Pad pads[4] = {
        { (Rectangle){ margin, margin, padW, padH }, (Color){ 60, 120, 220, 255 }, (Color){ 140, 190, 255, 255 }, KEY_A, {0}, 0.1f, 0.0f, "PAD 1 (A)" },
        { (Rectangle){ margin*2 + padW, margin, padW, padH }, (Color){ 220, 80, 80, 255 }, (Color){ 255, 160, 160, 255 }, KEY_S, {0}, 0.1f, 0.0f, "PAD 2 (S)" },
        { (Rectangle){ margin, margin*2 + padH, padW, padH }, (Color){ 80, 200, 120, 255 }, (Color){ 160, 255, 190, 255 }, KEY_D, {0}, 0.1f, 0.0f, "PAD 3 (D)" },
        { (Rectangle){ margin*2 + padW, margin*2 + padH, padW, padH }, (Color){ 240, 180, 60, 255 }, (Color){ 255, 220, 140, 255 }, KEY_F, {0}, 0.1f, 0.0f, "PAD 4 (F)" }
    };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        switch (currentState) {
            case STATE_MENU:
                if (IsKeyPressed(KEY_ENTER)) currentState = STATE_SELECT_KIT;
                break;

            case STATE_SELECT_KIT:
                if (IsKeyPressed(KEY_UP)) selectedKitIndex = (selectedKitIndex - 1 + 4) % 4;
                if (IsKeyPressed(KEY_DOWN)) selectedKitIndex = (selectedKitIndex + 1) % 4;
                
                if (IsKeyPressed(KEY_ENTER)) {
                    // Unload previous sounds if they exist
                    for (int i = 0; i < 4; i++) if (padSounds[i].frameCount > 0) UnloadSound(padSounds[i]);
                    
                    // Load the 4 sounds for the chosen section
                    for (int i = 0; i < 4; i++) {
                        padSounds[i] = LoadSound(kits[selectedKitIndex].samples[i]);
                        pads[i].sample = padSounds[i];
                    }
                    currentState = STATE_PLAYING;
                }
                break;

            case STATE_PLAYING:
                if (IsKeyPressed(KEY_P)) currentState = STATE_PAUSED;
                if (IsKeyPressed(KEY_BACKSPACE)) currentState = STATE_SELECT_KIT;

                for (int i = 0; i < 4; i++) {
                    if (IsKeyPressed(pads[i].key)) HitPad(&pads[i]);
                    if (pads[i].flashRemaining > 0.0f) pads[i].flashRemaining -= dt;
                }
                break;

            case STATE_PAUSED:
                if (IsKeyPressed(KEY_P)) currentState = STATE_PLAYING;
                break;
        }

        BeginDrawing();
        ClearBackground((Color){ 20, 20, 28, 255 });

        if (currentState == STATE_MENU) {
            DrawText("BEAT BOX PRO", screenWidth/2 - 120, 150, 40, RAYWHITE);
            DrawText("Press ENTER to Start", screenWidth/2 - 100, 220, 20, LIGHTGRAY);
        } 
        else if (currentState == STATE_SELECT_KIT) {
            DrawText("SELECT YOUR KIT", 50, 50, 30, RAYWHITE);
            for (int i = 0; i < 4; i++) {
                Color c = (i == selectedKitIndex) ? YELLOW : GRAY;
                DrawText(kits[i].kitName, 100, 120 + (i * 50), 25, c);
                if (i == selectedKitIndex) DrawText(">", 70, 120 + (i * 50), 25, YELLOW);
            }
            DrawText("Use ARROWS to move, ENTER to select", 50, 380, 20, DARKGRAY);
        }
        else {
            // Draw Drum Pads
            for (int i = 0; i < 4; i++) {
                Color c = (pads[i].flashRemaining > 0.0f) ? pads[i].hitColor : pads[i].baseColor;
                DrawRectangleRec(pads[i].rect, c);
                DrawText(pads[i].label, pads[i].rect.x + 20, pads[i].rect.y + 20, 20, BLACK);
            }
            DrawText(TextFormat("KIT: %s | P: Pause | BKSP: Change Kit", kits[selectedKitIndex].kitName), 20, screenHeight - 25, 18, RAYWHITE);

            if (currentState == STATE_PAUSED) {
                DrawRectangle(0, 0, screenWidth, screenHeight, (Color){ 0, 0, 0, 150 });
                DrawText("PAUSED", screenWidth/2 - 60, screenHeight/2 - 20, 40, RAYWHITE);
            }
        }
        EndDrawing();
    }

    // Final Cleanup
    for (int i = 0; i < 4; i++) if (padSounds[i].frameCount > 0) UnloadSound(padSounds[i]);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}