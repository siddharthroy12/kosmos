#include "platform.h"
#include "sokol_app.h"

platform_window_details window = {
    .width  = 800,
    .height = 450,
    .title  = "rayfork"
};


rf_texture2d texture;

extern void on_init()
{
    // Load texture
    texture = rf_load_texture_from_file_ez(ASSETS_PATH"test.png"); // _ez function will use libc's io and allocator
}

extern void on_update()
{
    rf_begin();
    {
        rf_clear(RF_RAYWHITE);

        int image_x = window.width / 2 - texture.width / 2;
        int image_y = window.height / 2 - texture.height / 2;
        rf_draw_texture(texture, image_x, image_y, RF_WHITE);

        char*    text = "Congrats! You created your first window!";
        rf_sizef size = rf_measure_text(rf_get_default_font(), text, 20, 2);
        rf_draw_text(text, window.width / 2 - size.width / 2, image_y - size.height - 20, 20, RF_BLACK);
    }
    rf_end();
}

extern void on_event(const sapp_event* event) {}