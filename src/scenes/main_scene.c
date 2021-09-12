#include "../scene.h"
#include <stdio.h>
#include "game_scene.h"
#include "../utils/button.h"
#include "raylib.h"

#ifndef GAME_VERSION
#define GAME_VERSION "N/A"
#endif

#define TITLE_FONT_SIZE 80

// Actions
static bool game_exit = false;
static bool start_game_pressed = false;
// Buttons
static button buttons[2] = { 0 };

// Mouse
Vector2 mouse_pos;

static void draw_title(Vector2 pos, char* title) {
	float width = MeasureText(title, TITLE_FONT_SIZE);
	DrawText(title, pos.x - (width /2), pos.y - (TITLE_FONT_SIZE /2), TITLE_FONT_SIZE, BLUE);
}

static void draw_version(void) {
	char *text = "Ver: "GAME_VERSION;
	int width = MeasureText(text, 20);
	DrawText(text, GetScreenWidth() - width - 20, GetScreenHeight() - 40, 20, BLUE);
}

static void draw_credit(void) {
	char *text = "@Siddharth_Roy12";
	DrawText(text, 20, GetScreenHeight() - 40, 20, BLUE);
}

static void exit_game(void) {
	game_exit = true;
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

static void event_handle(void) {
	if (IsKeyPressed(KEY_ESCAPE)) {
		exit_game();
	}

	if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
		start_game();
	}
}

static void on_scene_update(void(*change_scene)(scene *scn), bool *should_exit,  float delta) {
	BeginDrawing();
	
	ClearBackground((Color){18, 18, 18, 255});

	draw_title((Vector2){ GetScreenWidth()/ 2, (GetScreenHeight()/2) - 100 }, "Kosmos");

	update_and_draw_buttons(buttons, 2, GetMousePosition(),  (Vector2){ GetScreenWidth(), GetScreenHeight() });

	draw_version();
	draw_credit();
	EndDrawing();


	if (start_game_pressed) {
		change_scene(&game_scene);
	}

	if (game_exit) {
		*should_exit = true;
	}

	event_handle();
}


static void on_scene_exit(void) {
}

const scene main_scene = {
	.init_fn = on_scene_load,
	.update_fn = on_scene_update,
	.exit_fn = on_scene_exit,
};