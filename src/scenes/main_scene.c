#include "../scene.h"
#include "rayfork.h"
#include <stdio.h>
#include "game_scene.h"

#define TITLE_FONT_SIZE 40

typedef struct button {
	char *title;
	short offset;
	bool border;
	void (*on_click)(void);
} button;

// Actions
bool mouse_left_click = false;
bool start_game_pressed = false;

// Buttons
button buttons[2] = { 0 };

// Mouse
rf_vec2 mouse_pos;

rf_rec draw_button(button btn, bool hover) {
	rf_color color = hover ? RF_RED : RF_WHITE;

	float pos_x = (sapp_width()/2);
	float pos_y = (sapp_height()/2) + btn.offset;

	rf_rec rec = {
		.height = 40,
		.width = rf_measure_text(rf_get_default_font(), btn.title, 20, 2).width + 20,
		.x = pos_x - 10,
		.y = pos_y - 10
	};

	rec.x = (rec.x - rec.width / 2) + 10; 
	rec.y = (rec.y -rec.height / 2) + 10;

	if (btn.border) rf_draw_rectangle_outline(rec, 1, color);

	rf_draw_text(btn.title, rec.x + 10, rec.y + 10, 20, color);

	return rec;
}

void update_and_draw_buttons(void) {
	rf_rec button_rec = { 0 };
	
	for (int i = 0; i < 2; i++) {
		button_rec = draw_button(buttons[i], false);
		
		if (rf_check_collision_point_rec(mouse_pos, button_rec)) {
			draw_button(buttons[i], true);
		}

		if (mouse_left_click) {
			if (rf_check_collision_point_rec(mouse_pos, button_rec)) {
				buttons[i].on_click();
			}
		}
	}
}

void draw_title(rf_vec2 pos, char* title) {
	float width = rf_measure_text(rf_get_default_font(), title, TITLE_FONT_SIZE, 2).width;
	rf_draw_text(title, pos.x - (width /2), pos.y - (TITLE_FONT_SIZE /2), TITLE_FONT_SIZE, RF_BLUE);
}

void exit_game(void) {
	sapp_quit();
}

void start_game(void) {
	start_game_pressed = true;
}

static void on_scene_load(void) {
	// Generate buttons
	buttons[0] = (button){
		.title = "Start",
		.offset = 0,
		.border = true,
		.on_click = start_game
	};

	buttons[1] = (button) {
		.title = "Exit",
		.offset = 50,
		.border = false,
		.on_click = exit_game
	};
}

static void on_scene_update(void(*change_scene)(scene *scn)) {
	rf_clear((rf_color){18, 18, 18, 255});

	draw_title((rf_vec2){ sapp_width()/ 2, (sapp_height()/2) - 100 }, "Kosmos");

	update_and_draw_buttons();

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