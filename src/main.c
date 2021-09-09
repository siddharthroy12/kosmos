#include "platform.h"
#include "sokol_app.h"
#include "./scenes/main_scene.h"
#include "./scenes/game_scene.h"
#include "time.h"

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

// Delta

float delta = 0.0f;
uint64_t start_time = 0;

static void change_scene(scene *scn) {
    scene_to_change_to = scn;
    scene_will_change = true;
}


// Initialization
extern void game_init()
{
    srand(time(0));
    
    sapp_toggle_fullscreen();
    current_scene = load_scene(&main_scene);
    stm_setup();
}

// Main Loop
extern void game_update()
{
    start_time = stm_now();

    rf_begin();
        current_scene.update_fn(change_scene, delta);
    rf_end();

    if (scene_will_change) {
        switch_scene(&current_scene, scene_to_change_to);
        scene_will_change = false;
    }

    delta = stm_ms(stm_since(start_time));
}

// Event callback
extern void game_event(const sapp_event* event)
{
   current_scene.event_fn(event);

   switch (event->type) {
       case SAPP_EVENTTYPE_KEY_DOWN:
        switch (event->key_code) {
            case SAPP_KEYCODE_F11:
                sapp_toggle_fullscreen();
            break;
        }
        break;
   }
}

// On Exit
extern void game_exit(void)
{
    current_scene.exit_fn();
}