#ifndef BUTTON_H
#define BUTTON_H

#include "rayfork.h"

typedef struct button {
	char *title;
	short offset;
	bool border;
	bool pressed;
	void (*on_click)(void);
} button;

static rf_rec draw_button(button btn, bool hover, rf_vec2 size) {
	rf_color color = hover || btn.pressed ? RF_RED : RF_WHITE;

	float pos_x = (size.x/2);
	float pos_y = (size.y/2) + btn.offset;

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

static void update_and_draw_button(button *button, rf_vec2 mouse_pos, bool mouse_left_click, rf_vec2 size) {
	rf_rec button_rec = { 0 };
	button_rec = draw_button(*button, false, size);
		
	if (rf_check_collision_point_rec(mouse_pos, button_rec)) {
		draw_button(*button, true, size);
	}

	if (mouse_left_click) {
		if (rf_check_collision_point_rec(mouse_pos, button_rec)) {
			button->on_click();
		}
	}
}

static void update_and_draw_buttons(button *buttons, int length, rf_vec2 mouse_pos, bool mouse_left_click, rf_vec2 size) {
	for (int i = 0; i < length; i++) {
		update_and_draw_button(&buttons[i], mouse_pos, mouse_left_click, size);
	}
}

#endif