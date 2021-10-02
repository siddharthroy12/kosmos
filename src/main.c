#include <time.h>
#include "./scenes/main_scene.h"

#include "raylib.h"

#define WINDOW_HEIGHT 450
#define WINDOW_WIDTH 800

// Scene management
scene current_scene = { 0 };
bool scene_will_change = false;
scene *scene_to_change_to = { 0 };

// Global sound variable
Sound click_sound;

static void change_scene(scene *scn) {
    scene_to_change_to = scn;
    scene_will_change = true;
}

int main(void) {
    SetTraceLogLevel(LOG_NONE);
    SetWindowState(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Kosmos");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    InitAudioDevice();
    ToggleFullscreen();

    click_sound = LoadSound(ASSETS_PATH"audio/toggle_002.ogg");
   
    SetExitKey(0); // Do not exit on ESC key

    //ToggleFullscreen();

    current_scene = load_scene(&main_scene);

    bool should_exit = false;

    while (!should_exit && !WindowShouldClose()) {
        current_scene.update_fn(change_scene, &should_exit, GetFrameTime() * 1000);

        if (scene_will_change) {
            switch_scene(&current_scene, scene_to_change_to);
            scene_will_change = false;
        }
        
        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }
    }

    current_scene.exit_fn();
    CloseWindow();
    UnloadSound(click_sound);
    return 0;
}