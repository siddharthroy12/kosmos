#include "../scene.h"
#include "rayfork.h"
#include <stdio.h>
#include "game_scene.h"
#include "../utils/button.h"

#define TITLE_FONT_SIZE 80

// Actions
bool mouse_left_click = false;
bool start_game_pressed = false;

// Buttons
button buttons[2] = { 0 };

// Mouse
rf_vec2 mouse_pos;

void draw_title(rf_vec2 pos, char* title) {
	float width = rf_measure_text(rf_get_default_font(), title, TITLE_FONT_SIZE, 2).width;
	rf_draw_text(title, pos.x - (width /2), pos.y - (TITLE_FONT_SIZE /2), TITLE_FONT_SIZE, RF_BLUE);
}

static void exit_game(void) {
	sapp_quit();
}

static void start_game(void) {
	start_game_pressed = true;
}

static void on_scene_load(void) {
	// Generate buttons
	buttons[0] = (button){
		.title = "Start",
		.offset = 0,
		.border = true,
		.on_click = start_game,
		.pressed = false
	};

	buttons[1] = (button) {
		.title = "Exit",
		.offset = 50,
		.border = false,
		.on_click = exit_game,
		.pressed = false
	};
}

static void on_scene_update(void(*change_scene)(scene *scn), float delta) {
	rf_clear((rf_color){18, 18, 18, 255});

	draw_title((rf_vec2){ sapp_width()/ 2, (sapp_height()/2) - 100 }, "Kosmos");

	update_and_draw_buttons(buttons, 2, mouse_pos, mouse_left_click, (rf_vec2){ sapp_width(), sapp_height() });

	if (start_game_pressed) {
		change_scene(&game_scene);
	}
	
}

static void on_event(const sapp_event *event) {
	switch(event->type) {
		case SAPP_EVENTTYPE_MOUSE_DOWN:
			switch(event->mouse_button) {
				case SAPP_MOUSEBUTTON_LEFT:
					mouse_left_click = true;
					break;
				default:
					break;
			}
			break;
		case SAPP_EVENTTYPE_MOUSE_UP:
			switch(event->mouse_button) {
				case SAPP_MOUSEBUTTON_LEFT:
					mouse_left_click = false;
					break;
				default:
					break;
			}
			break;
		case SAPP_EVENTTYPE_MOUSE_MOVE:
			mouse_pos.x = event->mouse_x;
			mouse_pos.y = event->mouse_y;
			break;
		case SAPP_EVENTTYPE_KEY_DOWN:
			switch (event->key_code) {
				case SAPP_KEYCODE_ESCAPE:
					buttons[1].pressed = true;
					exit_game();
					break;
				case SAPP_KEYCODE_SPACE:
				case SAPP_KEYCODE_ENTER:
					buttons[2].pressed = true;
					start_game();
				default:
					break;
			}
		default:
			break;
		}
}


static void on_scene_exit(void) {
}

const scene main_scene = {
	.init_fn = on_scene_load,
	.update_fn = on_scene_update,
	.exit_fn = on_scene_exit,
	.event_fn = on_event
};