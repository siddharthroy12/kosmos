#include "platform.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#define SOKOL_WIN32_NO_GL_LOADER
#define SOKOL_WIN32_FORCE_MAIN
#include "glad/glad.h"
#include "sokol/sokol_app.h"

#ifdef RAYFORK_PLATFORM_MACOS
// On macos sokol app includes an opengl3 header which would collide with glad.h so we just declare the glad loader function which we need
extern int gladLoadGL(void);
#else
#include "glad/glad.h"
#endif

rf_context ctx;
rf_render_batch batch;

static void sokol_on_init(void)
{
    gladLoadGL();
    
    // Initialize rayfork
    rf_init_context(&ctx);
    rf_init_gfx(window.width, window.height, RF_DEFAULT_GFX_BACKEND_INIT_DATA);

    // Initialize the rendering batch
    batch = rf_create_default_render_batch(RF_DEFAULT_ALLOCATOR);
    rf_set_active_render_batch(&batch);

    on_init();
}

static void sokol_on_frame(void)
{
    on_update();
}

static void sokol_on_event(const sapp_event* event)
{
    switch (event->type)
    {
        case SAPP_EVENTTYPE_RESIZED:
            rf_set_viewport(event->window_width, event->window_height);
            break;
        default:
            break;
    }

    // Define this function in game.c
    on_event(event);
}

sapp_desc sokol_main(int argc, char** argv)
{
    return (sapp_desc)
    {
        .window_title = window.title,
        .width        = window.width,
        .height       = window.height,
        .init_cb      = sokol_on_init,
        .frame_cb     = sokol_on_frame,
        .event_cb     = sokol_on_event,
    };
}