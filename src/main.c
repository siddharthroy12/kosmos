#include "platform.h"
#include "sokol_app.h"
#include "./scenes/main_scene.h"
#include "./scenes/game_scene.h"

// Window Config
platform_window_details window = {
    .width  = 800,
    .height = 450,
    .title  = "Kosmos"
};

// Scene management
scene current_scene = { 0 };
bool scene_will_change = false;
scene *scene_to_change_to = { 0 };

static void change_scene(scene *scn) {
    scene_to_change_to = scn;
    scene_will_change = true;
}


// Initialization
extern void game_init()
{
    current_scene = load_scene(&main_scene);
}

// Main Loop
extern void game_update()
{
    rf_begin();
        current_scene.update_fn(change_scene);
    rf_end();

    if (scene_will_change) {
        switch_scene(&current_scene, scene_to_change_to);
        scene_will_change = false;
    }
}

// Event callback
extern void game_event(const sapp_event* event)
{
   current_scene.event_fn(event);
}

// On Exit
extern void game_exit(void)
{
    current_scene.exit_fn();
}