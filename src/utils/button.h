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
	Color color = hover || btn.pressed ? RED : WHITE;

	float pos_x = (size.x/2);
	float pos_y = (size.y/2) + btn.offset;

	Rectangle rec = {
		.height = 40,
		.width = MeasureTextEx(GetFontDefault(), btn.title, 20, 2).x + 20,
		.x = pos_x - 10,
		.y = pos_y - 10
	};

	rec.x = (rec.x - rec.width / 2) + 10; 
	rec.y = (rec.y -rec.height / 2) + 10;
	
	
	if (btn.border) DrawRectangleLinesEx(rec, 2, color);

	DrawText(btn.title, rec.x + 10, rec.y + 10, 20, color);

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