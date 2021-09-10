#ifndef BUTTON_H
#define BUTTON_H

#include "raylib.h"

typedef struct button {
	char *title;
	short offset;
	bool border;
	bool pressed;
	void (*on_click)(void);
} button;

static Rectangle draw_button(button btn, bool hover, Vector2 size) {
	#ifdef BIG_BUTTON
	int font_size = 40;
	int extra_width = 40;
	int height = 70;
	int offset = 15;
	#else
	int height = 40;
	int font_size = 20;
	int extra_width = 20;
	int offset = 10;
	#endif

	Color color = hover || btn.pressed ? RED : WHITE;

	float pos_x = (size.x/2);
	float pos_y = (size.y/2) + btn.offset;


	Rectangle rec = {
		.height = height,
		.width = MeasureTextEx(GetFontDefault(), btn.title, font_size, 2).x + extra_width,
		.x = pos_x,
		.y = pos_y
	};

	rec.x = (rec.x - rec.width / 2); 
	rec.y = (rec.y -rec.height / 2);

	
	
	if (btn.border) DrawRectangleLinesEx(rec, 3, color);

	DrawText(btn.title, rec.x + offset, rec.y + offset, font_size, color);

	return rec;
}

static void update_and_draw_button(button *button, Vector2 mouse_pos, Vector2 size) {
	Rectangle button_rec = { 0 };
	button_rec = draw_button(*button, false, size);
		
	
	if (CheckCollisionPointRec(mouse_pos, button_rec)) {
		draw_button(*button, true, size);
	}

	if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
		if (CheckCollisionPointRec(mouse_pos, button_rec)) {
			button->on_click();
		}
	}
}

static void update_and_draw_buttons(button *buttons, int length, Vector2 mouse_pos, Vector2 size) {
	for (int i = 0; i < length; i++) {
		update_and_draw_button(&buttons[i], mouse_pos, size);
	}
}

#endif