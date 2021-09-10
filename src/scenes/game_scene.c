#include "../scene.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#include "../utils/button.h"

scene game_scene;

#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))
#define rand_range(a, b) (rand() % (b + 1 - a) + a)

#define RENDER_WIDTH 1280
#define RENDER_HEIGHT 720

#define PLAYER_MAX_SPEED 0.4f
#define PLAYER_ACCLERATION 0.009f
#define PLAYER_DEACCLERATION 0.003f
#define PLAYER_HIT_RADIUS 10

#define BULLETS_BUFFER_SIZE 150
#define BULLET_SPEED 1.0f
#define BULLET_SIZE 3

#define ASTROIDS_BUFFER_SIZE 70
#define ASTROID_SPEED 0.009f
#define ASTROID_SIZE 50

#define ENEMIES_BUFFER_SIZE 10
#define ENEMY_MAX_HEALTH 10
#define ENEMY_RADAR_RANGE 500
#define ENEMY_HIT_RANGE 50

#define GAME_BORDER_RADIUS 2000

#define RENDER_DISTANCE 1500

#define OVERLAY_WINDOW_WIDTH 500
#define OVERLAY_WINDOW_HEIGHT 300

typedef struct bullet {
	Vector2 pos;
	Vector2 dir;
	bool visible;
	bool enemy;
} bullet;

typedef struct astroid {
	Vector2 pos;
	Vector2 dir;
	int segments;
} astroid;

typedef struct enemy {
	Vector2 pos;
	Vector2 dir;
	int health;
} enemy;

astroid astroid_buffer[ASTROIDS_BUFFER_SIZE] = { 0 };

bullet bullet_buffer[BULLETS_BUFFER_SIZE] = { 0 };
int current_bullet_buffer = 0;

enemy enemy_buffer[ENEMIES_BUFFER_SIZE] = { 0 };

// Mouse
Vector2 virtual_mouse_pos = { 0 };


// Player
Texture2D player_texture = { 0 };
Vector2 player_pos = { 0.0f, 0.0f };
Vector2 player_dir = { 0.0f, 0.0f };
float player_speed = 0.0f;
bool game_over = false;
bool game_pause = false;
static bool game_exit = false;

// Timer
double shoot_start_time;

// Render Texture
RenderTexture2D render_texture;

// Camera
Camera2D camera = {
	.target = (Vector2) { 0.0f ,0.0f },
	.offset = (Vector2){ (float)RENDER_WIDTH/2 ,(float) RENDER_HEIGHT/2 },
	.rotation = 0.0f,
	.zoom = 1.0f,
};

// Buttons
button buttons[2];

Vector2 clamp_value(Vector2 value, Vector2 min, Vector2 max)
{
    Vector2 result = value;
    result.x = (result.x > max.x)? max.x : result.x;
    result.x = (result.x < min.x)? min.x : result.x;
    result.y = (result.y > max.y)? max.y : result.y;
    result.y = (result.y < min.y)? min.y : result.y;
    return result;
}

void reset_state(void) {
	current_bullet_buffer = 0;
	player_pos = (Vector2){ 0.0f, 0.0f };
	player_dir = (Vector2){ 0.0f, 0.0f };
	player_speed = 0.0f;
	game_over = false;
	game_pause = false;

	// Init bullets
	for (int i = 0; i < BULLETS_BUFFER_SIZE; i++) {
		bullet_buffer[i] = (bullet){
			.dir = (Vector2){ 0.0f, 0.0f },
			.pos = (Vector2){ 0.0f, 0.0f },
			.visible = false,
			.enemy = false
		};
	}

	// Init Astroids
	for (int i = 0; i < ASTROIDS_BUFFER_SIZE; i++) {
		astroid_buffer[i] = (astroid){
			.dir = (Vector2){ 0.0f, 0.0f },
			.pos = (Vector2){ (float)rand_range(-GAME_BORDER_RADIUS, GAME_BORDER_RADIUS), (float)rand_range(-GAME_BORDER_RADIUS, GAME_BORDER_RADIUS) },
			.segments = rand_range(5, 10)
		};
	}

	// Init Enemies
	for (int i = 0; i < ENEMIES_BUFFER_SIZE; i++) {
		enemy_buffer[i] = (enemy){
			.pos = (Vector2){ (float)rand_range(-GAME_BORDER_RADIUS, GAME_BORDER_RADIUS), (float)rand_range(-GAME_BORDER_RADIUS, GAME_BORDER_RADIUS) },
			.dir = (Vector2){ 0.0f, 0.0f },
			.health = ENEMY_MAX_HEALTH
		};
	}
}

// Action functions
bool is_accelerate(void) {
	bool result = false;

	if (IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
		result = true;
	}

	return result;
}

bool is_shoot(void) {
	bool result = false;

	if (IsKeyDown(KEY_W) || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
		result = true;
	}

	return result;
}

void update_camera(void) {
	camera.target = player_pos;
}

void draw_and_update_enemies(float delta) {
	Vector2 end_point = { 0 };
	for (int i = 0; i < ENEMIES_BUFFER_SIZE; i++) {
		if (enemy_buffer[i].health > 0) {
			
			end_point = Vector2Add(enemy_buffer[i].pos, (Vector2){
				.x =  (Vector2Normalize(Vector2Subtract(enemy_buffer[i].dir, enemy_buffer[i].pos)).x * 80),
				.y =  (Vector2Normalize(Vector2Subtract(enemy_buffer[i].dir, enemy_buffer[i].pos)).y * 80),
			});

			if (!game_pause) {
				// Move the gun and shoot when player comes closer
				if (Vector2Distance(player_pos, enemy_buffer[i].pos) < ENEMY_RADAR_RANGE && !game_over) {
					enemy_buffer[i].dir.x = Lerp(enemy_buffer[i].dir.x, player_pos.x, delta * 0.002);
					enemy_buffer[i].dir.y = Lerp(enemy_buffer[i].dir.y, player_pos.y, delta * 0.002);

					bullet_buffer[current_bullet_buffer].enemy = true;
					bullet_buffer[current_bullet_buffer].visible = true;
					bullet_buffer[current_bullet_buffer].pos = enemy_buffer[i].pos;
					bullet_buffer[current_bullet_buffer].dir = Vector2Normalize(Vector2Subtract(bullet_buffer[current_bullet_buffer].pos, end_point));
					
					if (current_bullet_buffer == BULLETS_BUFFER_SIZE - 1) {
						current_bullet_buffer = 0;
					} else {
						current_bullet_buffer++;
					}
				} 

				// Check player collision
				if (CheckCollisionCircles(enemy_buffer[i].pos, ENEMY_HIT_RANGE, player_pos, PLAYER_HIT_RADIUS)) {
					game_over = true;
				}

				// Check Player Bullet collision
				for (int j = 0; j < BULLETS_BUFFER_SIZE; j++) {
					if (CheckCollisionCircles(enemy_buffer[i].pos, ENEMY_HIT_RANGE, bullet_buffer[j].pos, BULLET_SIZE)) {
						if (bullet_buffer[j].visible && !bullet_buffer[j].enemy) {
							enemy_buffer[i].health--;
							bullet_buffer[j].visible = false;
						}
						
					}
				}
			}

			if (CheckCollisionCircles(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, player_pos, RENDER_DISTANCE)) {
				DrawRing(enemy_buffer[i].pos, ASTROID_SIZE, ASTROID_SIZE +3, 0,360, 5, RED);
				Color inside = RED;
				inside.a = 200;
				DrawCircleSector(enemy_buffer[i].pos, ASTROID_SIZE, 0,360, 5, inside);

				// Draw rader range
				inside = BLUE;
				inside.a = 5;
				DrawCircleSector(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, 0, 360, 100, inside);
				DrawRing(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, ENEMY_RADAR_RANGE+3, 0, 360, 100, SKYBLUE);

				// Draw gun
				DrawLineEx(enemy_buffer[i].pos, end_point , 10, LIME);

				// Draw Health
				DrawText(TextFormat("HEALTH: %i", enemy_buffer[i].health), enemy_buffer[i].pos.x - 30, enemy_buffer[i].pos.y - 80, 20, YELLOW);
			}
		}
	}
}

void draw_and_update_player(float delta) {
	Vector2 world_mouse_pos = GetScreenToWorld2D(virtual_mouse_pos, camera);
	float rotation = Vector2Angle(player_pos, world_mouse_pos) + 90.0f;

	// Draw player sprite
	DrawTexturePro(
		player_texture,
		(Rectangle){
			.x = 0,
			.y = 0,
			.width = player_texture.width,
			.height = player_texture.height
		},
		(Rectangle){
			.x = player_pos.x,
			.y = player_pos.y,
			.width = 20,
			.height = 20,
		},
		(Vector2){ 10, 10 },
		rotation,
		WHITE
	);

	if (!game_pause) {
		// If cursor if away enough and player hasn't reached his max speed then accelerate or else deaccelerate
		if (is_accelerate() && player_speed < PLAYER_MAX_SPEED && Vector2Distance(world_mouse_pos, player_pos) > 5.0f && !game_over) {
			player_speed += PLAYER_ACCLERATION * delta;
		} else {
			player_speed = Lerp(player_speed, 0.0f, PLAYER_DEACCLERATION * delta);
		}

		// Get the direction from player to cursor
		player_dir = Vector2Normalize(Vector2Subtract(player_pos, world_mouse_pos));

		// Move the player towards direction
		if (Vector2Distance(world_mouse_pos, player_pos) > 5.0f) {
			player_pos = Vector2Add(player_pos, (Vector2){
				.x = -(player_dir.x * player_speed * delta),
				.y = -(player_dir.y * player_speed * delta)
			});
		}

		// Don't let player cross the border
		float distance = Vector2Distance((Vector2){ 0.0f, 0.0f}, player_pos);

		if (distance > GAME_BORDER_RADIUS) {
			DrawText("Don't Cross the border", player_pos.x, player_pos.y - 40.0f, 20, RED);
			Vector2 origin_to_player = Vector2Normalize(Vector2Subtract(player_pos, (Vector2){0.0f,0.0f}));
			player_pos = Vector2Scale(origin_to_player, GAME_BORDER_RADIUS);
		}

		// Check astroid collision
		for (int i = 0; i < ASTROIDS_BUFFER_SIZE; i++) {
			if (CheckCollisionCircles(astroid_buffer[i].pos, ASTROID_SIZE, player_pos, PLAYER_HIT_RADIUS)) {
				game_over = true;
			}
		}

		// Check bullet collision
		for (int i = 0; i < BULLETS_BUFFER_SIZE; i++) {
			if (CheckCollisionCircles(bullet_buffer[i].pos, BULLET_SIZE, player_pos, PLAYER_HIT_RADIUS)) {
				if (bullet_buffer[i].visible && bullet_buffer[i].enemy)
					game_over = true;
			}
		}

		// Handle shoot logic
		if (is_shoot() && ((GetTime() - shoot_start_time) >  0.2) && !game_over) {
			bullet_buffer[current_bullet_buffer].enemy = false;
			bullet_buffer[current_bullet_buffer].visible = true;
			bullet_buffer[current_bullet_buffer].pos = player_pos;
			bullet_buffer[current_bullet_buffer].dir = Vector2Normalize(Vector2Subtract(player_pos, world_mouse_pos));
			
			if (current_bullet_buffer == BULLETS_BUFFER_SIZE - 1) {
				current_bullet_buffer = 0;
			} else {
				current_bullet_buffer++;
			}

			shoot_start_time = GetTime();
		}
	}


	// Draw render distance border
	DrawCircleLines(player_pos.x, player_pos.y, RENDER_DISTANCE, RED);

	// Draw his radius
	//rf_draw_circle_lines(player_pos.x ,player_pos.y, PLAYER_HIT_RADIUS, RF_GREEN);
}

void draw_and_update_bullets(float delta) {
	for (int i = 0; i < BULLETS_BUFFER_SIZE; i++) {
		if (bullet_buffer[i].visible) {
			DrawCircle(bullet_buffer[i].pos.x, bullet_buffer[i].pos.y, BULLET_SIZE, YELLOW);
		}

		if (!game_pause) {
			bullet_buffer[i].pos = Vector2Add(bullet_buffer[i].pos, (Vector2){
				.x = -(bullet_buffer[i].dir.x * BULLET_SPEED * delta),
				.y = -(bullet_buffer[i].dir.y * BULLET_SPEED * delta)
			});

			for (int j = 0; j < ASTROIDS_BUFFER_SIZE; j++) {
				if (CheckCollisionCircles(bullet_buffer[i].pos, BULLET_SIZE, astroid_buffer[j].pos, ASTROID_SIZE)) {
					bullet_buffer[i].visible = false;
				}
			}
		}
		
	}
}

void draw_and_update_astroids(float delta) {
	for (int i = 0; i < ASTROIDS_BUFFER_SIZE; i++) {
		astroid_buffer[i].pos = Vector2Add(astroid_buffer[i].pos, (Vector2){
			.x = -(astroid_buffer[i].dir.x * ASTROID_SPEED * delta),
			.y = -(astroid_buffer[i].dir.y * ASTROID_SPEED * delta)
		});

		if (Vector2Distance(astroid_buffer[i].pos, player_pos) < RENDER_DISTANCE) {
			//rf_draw_circle_lines(astroid_buffer[i].pos.x, astroid_buffer[i].pos.y, ASTROID_SIZE, RF_GRAY);
			DrawRing(astroid_buffer[i].pos, ASTROID_SIZE, ASTROID_SIZE +3, 0,360, astroid_buffer[i].segments, GRAY);
			Color inside = GRAY;
			inside.a = 200;
			DrawCircleSector(astroid_buffer[i].pos, ASTROID_SIZE, 0,360, astroid_buffer[i].segments, inside);
			//rf_draw_circle(astroid_buffer[i].pos.x, astroid_buffer[i].pos.y, ASTROID_SIZE, inside);
		}
	}
}

void draw_window(Color color) {
	Rectangle rec = {
		.x = (RENDER_WIDTH/2) - (OVERLAY_WINDOW_WIDTH/2),
		.y = (RENDER_HEIGHT/2) - (OVERLAY_WINDOW_HEIGHT/2),
		.width = OVERLAY_WINDOW_WIDTH,
		.height = OVERLAY_WINDOW_HEIGHT,
	};

	Color background_color = color;
	background_color.a = 50;

	DrawRectangleRec(rec, (Color){18, 18, 18, 255});
	DrawRectangleRec(rec, background_color);
	DrawRectangleLinesEx(rec, 3, color);
}

void draw_game_over(void) {
	draw_window(BLUE);

	char *text = "GAME OVER";
	int pos_x = (RENDER_WIDTH/2) - (MeasureTextEx(GetFontDefault(), text, 50, 5).x/2);
	int pos_y = (RENDER_HEIGHT/2) - 100;

	DrawText(text, pos_x, pos_y, 50, RED);

	update_and_draw_button(&buttons[0], virtual_mouse_pos, (Vector2){ RENDER_WIDTH, RENDER_HEIGHT });
	update_and_draw_button(&buttons[1], virtual_mouse_pos, (Vector2){ RENDER_WIDTH, RENDER_HEIGHT });
}

void draw_game_pause() {
	draw_window(GREEN);

	char *text = "GAME PAUSED";
	char *hint = "Press [ESC] to Resume";

	int pos_x = (RENDER_WIDTH/2) - (MeasureTextEx(GetFontDefault(), text, 50, 5).x/2);
	int pos_y = (RENDER_HEIGHT/2) - 100;

	int hint_x = (RENDER_WIDTH/2) - (MeasureTextEx(GetFontDefault(), text, 20, 10).x/2);

	DrawText(text, pos_x, pos_y, 50, RED);
	DrawText(hint, hint_x, pos_y + 400, 20, BLUE);
	update_and_draw_button(&buttons[0], virtual_mouse_pos, (Vector2){ RENDER_WIDTH, RENDER_HEIGHT });
	update_and_draw_button(&buttons[1], virtual_mouse_pos, (Vector2){ RENDER_WIDTH, RENDER_HEIGHT });
}

static void retry(void) {
	reset_state();
}

static void exit_game(void) {
	game_exit = true;
}


void handle_input(void) {
	if (IsKeyPressed(KEY_ESCAPE)) {
		if (!game_over)
			game_pause = !game_pause;
	}
}

static void on_scene_load(void) {
	HideCursor();

	// Render texture initialization, used to hold the rendering result so we can easily resize it
	render_texture = LoadRenderTexture(RENDER_WIDTH, RENDER_HEIGHT);
	SetTextureFilter(render_texture.texture, FILTER_BILINEAR);

	player_texture = LoadTexture(ASSETS_PATH"player.png");

	reset_state();

	// Init Buttons
	buttons[0] = (button){
		.title = "Restart",
		.offset = 0,
		.border = true,
		.on_click = retry,
		.pressed = false
	};

	buttons[1] = (button){
		.title = "Exit Game",
		.offset = 70,
		.border = true,
		.on_click = exit_game,
		.pressed = false
	};
}

static void on_scene_update(void(*change_scene)(scene *scn), bool *should_exit, float delta) {

	if (game_exit) {
		*should_exit = true;
	}

	// Compute required framebuffer scaling
    float scale = min((float)GetScreenWidth()/RENDER_WIDTH, (float)GetScreenHeight()/RENDER_HEIGHT);

	// Update virtual mouse (clamped mouse value behind game screen)
    virtual_mouse_pos.x = (GetMouseX() - (GetScreenWidth() - (RENDER_WIDTH*scale))*0.5f)/scale;
    virtual_mouse_pos.y = (GetMouseY() - (GetScreenHeight() - (RENDER_HEIGHT*scale))*0.5f)/scale;
    virtual_mouse_pos = clamp_value(virtual_mouse_pos, (Vector2){ 0, 0 }, (Vector2){ (float)RENDER_WIDTH, (float)RENDER_HEIGHT });

	BeginTextureMode(render_texture);
		ClearBackground((Color){18, 18, 18, 255});

		BeginMode2D(camera);
			draw_and_update_player(delta);

			draw_and_update_bullets(delta);

			draw_and_update_astroids(delta);

			draw_and_update_enemies(delta);

			// Draw game border
			DrawRing((Vector2){0.0f, 0.0f}, GAME_BORDER_RADIUS, GAME_BORDER_RADIUS+5, 0, 360, 150, GOLD);

			update_camera();

		EndMode2D();

		if (game_over) {
			DrawText("Man, You suck!", 20, RENDER_HEIGHT - 40, 20, RED);
			draw_game_over();
		} else {
			DrawText("Destory enemies and avoid astroids", 20, RENDER_HEIGHT - 40, 20, RED);
		}

		if (game_pause) {
			draw_game_pause();
		}

		// Draw cursor
		DrawCircle(virtual_mouse_pos.x, virtual_mouse_pos.y, 2, WHITE);
	EndTextureMode();

	BeginDrawing();
		ClearBackground(BLACK);
		// Draw render texture to screen, properly scaled

		DrawTexturePro(
			render_texture.texture,
			(Rectangle){ 0.0f, 0.0f, (float)render_texture.texture.width, (float)-render_texture.texture.height },
			(Rectangle){
				(GetScreenWidth() - ((float)RENDER_WIDTH*scale))*0.5f,
				(GetScreenHeight() - ((float)RENDER_HEIGHT*scale))*0.5f,
				(float)RENDER_WIDTH*scale,
				(float)RENDER_HEIGHT*scale
			},
			(Vector2){ 0, 0 },
			0.0f,
			WHITE
		);

		DrawText(FormatText("FPS: %i", GetFPS()), 10, 10, 20, RED);

    EndDrawing();

	handle_input();

}

static void on_scene_exit(void) {
	printf("game Scene exit\n");
}

scene game_scene = {
	.init_fn = on_scene_load,
	.update_fn = on_scene_update,
	.exit_fn = on_scene_exit,
};