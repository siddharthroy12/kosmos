#ifndef PLATFORM_H
#define PLATFORM_H

#include "rayfork.h"
#include "sokol_app.h"
#include "sokol_time.h"

typedef struct platform_window_details
{
    int width;
    int height;
    const char* title;
} platform_window_details;

// The game program must define these functions and global variables
extern platform_window_details window;
extern void game_init(void);
extern void game_update(void);
extern void game_event(const sapp_event* event);
extern void game_exit(void);
#endif // PLATFORM_H