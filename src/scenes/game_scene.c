#include "../scene.h"
#include "rayfork.h"
#include <stdio.h>

// Mouse
rf_vec2 mouse_pos = { 0 };

static void on_scene_load(void) {
	printf("game Scene loaded\n");
	sapp_show_mouse(false);
}

static void on_scene_update(void(*change_scene)(scene *scn)) {
	rf_clear((rf_color){18, 18, 18, 255});
	rf_draw_text("Destory enemies and avoid astroids", 20, 20, 20, RF_RED);

	rf_draw_circle(mouse_pos.x, mouse_pos.y, 2, RF_WHITE);
}

static void on_event(const sapp_event *event) {
	switch (event->type) {
		case SAPP_EVENTTYPE_MOUSE_MOVE:
			mouse_pos.x = event->mouse_x;
			mouse_pos.y = event->mouse_y;
			break;
		default:
			break;
	}
}

static void on_scene_exit(void) {
	printf("game Scene exit\n");
}



const scene game_scene = {
	.init_fn = on_scene_load,
	.update_fn = on_scene_update,
	.exit_fn = on_scene_exit,
	.event_fn = on_event
};