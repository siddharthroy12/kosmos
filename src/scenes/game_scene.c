#include "../scene.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>
#define BIG_BUTTON
#include "../utils/button.h"

scene game_scene;

#define max(a, b) ((a)>(b)? (a) : (b))
#define min(a, b) ((a)<(b)? (a) : (b))
#define rand_range(a, b) (rand() % (b + 1 - a) + a)

#define RENDER_WIDTH 1366
#define RENDER_HEIGHT 768

#define PLAYER_MAX_SPEED 0.4f
#define PLAYER_ACCLERATION 0.009f
#define PLAYER_DEACCLERATION 0.003f
#define PLAYER_HIT_RADIUS 20
#define PLAYER_SIZE 50
#define PLAYER_SHOOT_TIME 0.2
#define PLAYER_MAX_HEALTH 5
#define PLAYER_SHIELD_RAD 60

#define BULLETS_BUFFER_SIZE 200
#define BULLET_SPEED 1.0f
#define BULLET_SIZE 3

#define ASTROIDS_BUFFER_SIZE 70
#define ASTROID_SPEED 0.009f
#define ASTROID_SIZE 50

#define ENEMIES_BUFFER_SIZE 10
#define ENEMY_MAX_HEALTH 5
#define ENEMY_RADAR_RANGE 800
#define ENEMY_SHOOT_TIME 0.15
#define ENEMY_HIT_RANGE 50

#define PICKUP_SIZE 30

#define GAME_BORDER_RADIUS 3000
#define SPAWN_RADIUS (int)(((GAME_BORDER_RADIUS * 2) / sqrt(2))/2)

#define RENDER_DISTANCE 1500

#define OVERLAY_WINDOW_WIDTH 650
#define OVERLAY_WINDOW_HEIGHT 380

#define IN_GAME_FONT_SIZE 25

#define SOUND_VOLUME 0.5f

typedef struct bullet {
	Vector2 pos;
	Vector2 dir;
	bool visible;
	bool enemy;
} bullet;

typedef struct astroid {
	Vector2 pos;
	Vector2 dir;
	float rotation;
} astroid;

typedef struct enemy {
	Vector2 pos;
	Vector2 dir;
	int health;
	double shoot_start_time;
} enemy;

typedef struct pickup {
	Vector2 pos;
	bool shown;
} pickup;

static astroid astroid_buffer[ASTROIDS_BUFFER_SIZE] = { 0 };
static Texture2D astroid_texture = { 0 };

static bullet bullet_buffer[BULLETS_BUFFER_SIZE] = { 0 };
static int current_bullet_buffer = 0;

static enemy enemy_buffer[ENEMIES_BUFFER_SIZE] = { 0 };
static int enemies_left = ENEMIES_BUFFER_SIZE;
static Texture2D enemy_texture = { 0 };

static pickup health_pickups[ENEMIES_BUFFER_SIZE] = { 0 };
static int current_health_pickup_buffer = 0;

static pickup shield_pickups[ENEMIES_BUFFER_SIZE] = { 0 };
static int current_shield_pickup_buffer = 0;

// Mouse
static Vector2 virtual_mouse_pos = { 0 };

// Player
static Texture2D cursor_texture = { 0 };
static Texture2D player_texture = { 0 };
static Vector2 player_pos = { 0.0f, 0.0f };
static Vector2 player_dir = { 0.0f, 0.0f };
static int player_health = PLAYER_MAX_HEALTH;
static float player_speed = 0.0f;
static float player_shield_on = true;
static float player_shield_time = 3;
static bool game_over = false;
static bool game_pause = false;
static bool game_exit = false;
static bool player_border_touch = false;

// Timer
static double shoot_start_time;
static double player_shield_start_time;
static double game_start_time;
static double game_over_time;

// Render Texture
static RenderTexture2D render_texture;

// Camera
static Camera2D camera = {
	.target = (Vector2) { 0.0f ,0.0f },
	.offset = (Vector2){ (float)RENDER_WIDTH/2 ,(float) RENDER_HEIGHT/2 },
	.rotation = 0.0f,
	.zoom = 0.8f,
};

// Buttons
static button buttons[2];

// State
static bool show_debug_info = false;
static bool won = false;
static float previous_score;

// Music
static Sound shoot_sound = { 0 };
static Sound enemy_shoot_sound = { 0 };
static Sound game_over_sound = { 0 };
static Sound enemy_damage_sound = { 0 };
static Sound enemy_death_sound = { 0 };
static Sound player_damage_sound = { 0 };
static Sound pickup_sound = { 0 };

static void load_sounds(void) {
	shoot_sound = LoadSound(ASSETS_PATH"audio/laserRetro_004.ogg");
	SetSoundVolume(shoot_sound, SOUND_VOLUME);
	enemy_shoot_sound = LoadSound(ASSETS_PATH"audio/laserSmall_001.ogg");
	SetSoundVolume(enemy_shoot_sound, SOUND_VOLUME);
	game_over_sound = LoadSound(ASSETS_PATH"audio/game_over.wav");
	SetSoundVolume(game_over_sound, SOUND_VOLUME);
	enemy_damage_sound = LoadSound(ASSETS_PATH"audio/enemy_damage.wav");
	SetSoundVolume(enemy_damage_sound, SOUND_VOLUME);
	enemy_death_sound = LoadSound(ASSETS_PATH"audio/explosionCrunch_000.ogg");
	SetSoundVolume(enemy_death_sound, SOUND_VOLUME);
	player_damage_sound = LoadSound(ASSETS_PATH"audio/player_damage.wav");
	SetSoundVolume(player_damage_sound, SOUND_VOLUME);
	pickup_sound = LoadSound(ASSETS_PATH"audio/confirmation_002.ogg");
	SetSoundVolume(pickup_sound, SOUND_VOLUME);
}

static void unload_sounds(void) {
	UnloadSound(shoot_sound);
	UnloadSound(enemy_shoot_sound);
	UnloadSound(game_over_sound);
	UnloadSound(enemy_damage_sound);
	UnloadSound(enemy_death_sound);
	UnloadSound(player_damage_sound);
	UnloadSound(pickup_sound);
}

static Vector2 clamp_value(Vector2 value, Vector2 min, Vector2 max) {
    Vector2 result = value;
    result.x = (result.x > max.x)? max.x : result.x;
    result.x = (result.x < min.x)? min.x : result.x;
    result.y = (result.y > max.y)? max.y : result.y;
    result.y = (result.y < min.y)? min.y : result.y;
    return result;
}

static void reset_state(void) {
	current_bullet_buffer = 0;
	player_pos = (Vector2){ 0.0f, 0.0f };
	player_dir = (Vector2){ 0.0f, 0.0f };
	player_speed = 0.0f;
	player_health = PLAYER_MAX_HEALTH;
	player_shield_on = true;
	player_shield_start_time = GetTime();
	enemies_left = ENEMIES_BUFFER_SIZE;
	game_over = false;
	game_pause = false;
	game_start_time = GetTime();
	won = false;
	
	if (LoadStorageValue(0) == 0) {
		previous_score = 1000.0f;
	} else {
		previous_score = (float)LoadStorageValue(0)/(float)1000;
	}

	// Init bullets
	for (int i = 0; i < BULLETS_BUFFER_SIZE; i++) {
		bullet_buffer[i] = (bullet){
			.dir = (Vector2){ 0.0f, 0.0f },
			.pos = (Vector2){ 0.0f, 0.0f },
			.visible = false,
			.enemy = false,
		};
	}

	// Init Astroids
	for (int i = 0; i < ASTROIDS_BUFFER_SIZE; i++) {
		astroid_buffer[i] = (astroid){
			.dir = (Vector2){ 0.0f, 0.0f },
			.pos = (Vector2){ (float)rand_range(-SPAWN_RADIUS, SPAWN_RADIUS), (float)rand_range(-SPAWN_RADIUS, SPAWN_RADIUS) },
			.rotation = (float)rand_range(0, 360)
		};
	}

	// Init Enemies
	for (int i = 0; i < ENEMIES_BUFFER_SIZE; i++) {
		enemy_buffer[i] = (enemy){
			.pos = (Vector2){ (float)rand_range(-SPAWN_RADIUS, SPAWN_RADIUS), (float)rand_range(-SPAWN_RADIUS, SPAWN_RADIUS) },
			.dir = (Vector2){ 0.0f, 0.0f },
			.health = ENEMY_MAX_HEALTH,
			.shoot_start_time = 0.0f,
		};
	}

	// Init Health and shield Pickups
	for (int i = 0; i < ENEMIES_BUFFER_SIZE; i++) {
		health_pickups[i] = (pickup){
			.pos = (Vector2){ 0.0f, 0.0f },
			.shown = false,
		};

		shield_pickups[i] = (pickup){
			.pos = (Vector2){ 0.0f, 0.0f },
			.shown = false,
		};
	}
}

// Action functions
static bool is_accelerate(void) {
	bool result = false;

	if (IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) {
		result = true;
	}

	return result;
}

static bool is_shoot(void) {
	bool result = false;

	if (IsKeyDown(KEY_W) || IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
		result = true;
	}

	return result;
}

static void set_game_over(void) {
	if (enemies_left) {
		player_health = 0;
		game_over = true;
		PlaySound(game_over_sound);
	}
}

static void update_camera(void) {
	camera.target = player_pos;
}

static void spawn_shield_pickup(void) {
	Vector2 shield_pickup_pos = (Vector2){ 
		.x = (float)rand_range(-SPAWN_RADIUS, SPAWN_RADIUS),
		.y = (float)rand_range(-SPAWN_RADIUS, SPAWN_RADIUS)
	};

	shield_pickups[current_shield_pickup_buffer].pos = shield_pickup_pos;
	shield_pickups[current_shield_pickup_buffer].shown = true;

	current_shield_pickup_buffer++;

	if (current_shield_pickup_buffer == ENEMIES_BUFFER_SIZE) {
		current_shield_pickup_buffer = 0;
	}

}

static void draw_cool_circle(Vector2 pos, int speed, int length, int gap, float size, Color color) {

	for (int i = 0; i < 360 / (gap+length); i++) {
		DrawRing(pos, size, size + 3, ((gap+length)*i) + (GetTime()*speed), length + ((gap+length)*i)  + (GetTime()*speed), 100, color);
	}
	
}

static void draw_and_update_enemies(float delta) {
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

					if (((GetTime() - enemy_buffer[i].shoot_start_time) >  ENEMY_SHOOT_TIME)) {
						enemy_buffer[i].shoot_start_time = GetTime();
						PlaySound(enemy_shoot_sound);
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
				} 

				// Check player to enemy collision
				if (CheckCollisionCircles(enemy_buffer[i].pos, ENEMY_HIT_RANGE, player_pos, PLAYER_HIT_RADIUS) && !game_over) {
					if (player_shield_on) {
						Vector2 direction_to_move = Vector2Normalize(Vector2Subtract(player_pos, enemy_buffer[i].pos));
						player_pos = Vector2Add(enemy_buffer[i].pos, Vector2Scale(direction_to_move, ENEMY_HIT_RANGE + PLAYER_HIT_RADIUS));
					} else {
						set_game_over();
					}
					
				}

				// Check Player Bullet  collision
				for (int j = 0; j < BULLETS_BUFFER_SIZE; j++) {
					// Check player bullet to enemy collision
					if (CheckCollisionCircles(enemy_buffer[i].pos, ENEMY_HIT_RANGE, bullet_buffer[j].pos, BULLET_SIZE)) {
						if (bullet_buffer[j].visible && !bullet_buffer[j].enemy) {
							PlaySound(enemy_damage_sound);
							enemy_buffer[i].health--;
							if (!enemy_buffer[i].health) {
								enemies_left--;
								PlaySound(enemy_death_sound);
								spawn_shield_pickup();
								health_pickups[current_health_pickup_buffer].pos = enemy_buffer[i].pos;
								health_pickups[current_health_pickup_buffer].shown = true;
								current_health_pickup_buffer++;
								if (current_health_pickup_buffer == ENEMIES_BUFFER_SIZE) {
									current_health_pickup_buffer = 0;
								}
							}
							bullet_buffer[j].visible = false;
						}
						
					}

					// Check player bullet to shield collision
					if (CheckCollisionCircles(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, bullet_buffer[j].pos, BULLET_SIZE) && !bullet_buffer[j].enemy) {
						if (!(Vector2Distance(player_pos, enemy_buffer[i].pos) < ENEMY_RADAR_RANGE))
							bullet_buffer[j].visible = false;
					}
				}
			}

			if (show_debug_info) {
				DrawRing(enemy_buffer[i].pos, ENEMY_HIT_RANGE, ENEMY_HIT_RANGE +2, 0, 360, 100, GREEN);
			}

			if (CheckCollisionCircles(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, player_pos, RENDER_DISTANCE)) {
				//DrawRing(enemy_buffer[i].pos, ASTROID_SIZE, ASTROID_SIZE +3, 0,360, 5, RED);
				Color inside = RED;
				inside.a = 100;
				DrawTexturePro(
					enemy_texture,
					(Rectangle){
						.x = 0.0f,
						.y = 0.0f,
						.width = enemy_texture.width,
						.height = enemy_texture.height
					},
					(Rectangle){
						.x = enemy_buffer[i].pos.x,
						.y = enemy_buffer[i].pos.y,
						.width = (ASTROID_SIZE+50),
						.height = (ASTROID_SIZE+50),
					},
					(Vector2){ (ASTROID_SIZE+50)/2, (ASTROID_SIZE+50)/2 },
					0.0f,
					WHITE
				);
				
				//DrawCircleSector(enemy_buffer[i].pos, ASTROID_SIZE, 0,360, 5, inside);

				// Draw rader range
				inside = BLUE;
				inside.a = 5;
				DrawCircleSector(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, 0, 360, 100, inside);
				
				// Enemy shield
				if (!(Vector2Distance(player_pos, enemy_buffer[i].pos) < ENEMY_RADAR_RANGE)) {
					DrawRing(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, ENEMY_RADAR_RANGE+3, 0, 360, 100, SKYBLUE);
				} else {
					draw_cool_circle(enemy_buffer[i].pos, 20, 15, 5, ENEMY_RADAR_RANGE, SKYBLUE );
				}

				// Draw gun
				DrawLineEx(enemy_buffer[i].pos, end_point , 7, WHITE);

				// Draw Health
				DrawRectanglePro(
					(Rectangle){
						enemy_buffer[i].pos.x,
						enemy_buffer[i].pos.y - 80,
						(100/ENEMY_MAX_HEALTH) * enemy_buffer[i].health,
						10
					},
					(Vector2){ 100/2, 10/2 },
					0.0f,
					RED
				);
				DrawRectangleLinesEx(
					(Rectangle){
						enemy_buffer[i].pos.x - 50,
						enemy_buffer[i].pos.y - 80,
						100,
						10
					},
					2,
					RED
				);
				//DrawText(TextFormat("HEALTH: %i", enemy_buffer[i].health), enemy_buffer[i].pos.x - 50, enemy_buffer[i].pos.y - 80, 20, RED);
			}
		}
	}
}

static void draw_and_update_player(float delta) {
	Vector2 world_mouse_pos = GetScreenToWorld2D(virtual_mouse_pos, camera);
	float rotation = Vector2Angle(player_pos, world_mouse_pos) + 90.0f;

	// Draw player
	Color green_background = LIME;
	green_background.a = 100;

	// Draw shield
	if (player_shield_on) {
		DrawCircle(player_pos.x, player_pos.y, PLAYER_SHIELD_RAD, green_background);
		draw_cool_circle(player_pos, 50, 20, 10, PLAYER_SHIELD_RAD, LIME);
		//DrawRing((Vector2){ player_pos.x, player_pos.y}, PLAYER_SHIELD_RAD, PLAYER_SHIELD_RAD + 3, 0, 360, 100, LIME);
	}

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
			.width = PLAYER_SIZE,
			.height = PLAYER_SIZE,
		},
		(Vector2){ PLAYER_SIZE/2, PLAYER_SIZE/2 },
		rotation,
		WHITE
	);

	if (show_debug_info) {
		DrawRing(player_pos, PLAYER_HIT_RADIUS, PLAYER_HIT_RADIUS + 2, 0, 360, 100, GREEN);
	}

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
			player_border_touch = true;
			Vector2 origin_to_player = Vector2Normalize(Vector2Subtract(player_pos, (Vector2){0.0f,0.0f}));
			player_pos = Vector2Scale(origin_to_player, GAME_BORDER_RADIUS);
		} else {
			player_border_touch = false;
		}

		// Check astroid collision
		for (int i = 0; i < ASTROIDS_BUFFER_SIZE; i++) {
			if (CheckCollisionCircles(astroid_buffer[i].pos, ASTROID_SIZE, player_pos, PLAYER_HIT_RADIUS) && !game_over) {
				if (player_shield_on) {
					Vector2 direction_to_move = Vector2Normalize(Vector2Subtract(player_pos, astroid_buffer[i].pos));
					player_pos = Vector2Add(astroid_buffer[i].pos, Vector2Scale(direction_to_move, ASTROID_SIZE + PLAYER_HIT_RADIUS));
				} else {
					set_game_over();
				}
			}
		}

		// Check bullet collision
		for (int i = 0; i < BULLETS_BUFFER_SIZE; i++) {
			if (CheckCollisionCircles(bullet_buffer[i].pos, BULLET_SIZE, player_pos, PLAYER_HIT_RADIUS) && !game_over) {
				if (bullet_buffer[i].visible && bullet_buffer[i].enemy) {
					PlaySound(player_damage_sound);
					player_health--;
				}
			}
		}

		// Handle shoot logic
		if (is_shoot() && ((GetTime() - shoot_start_time) >  PLAYER_SHOOT_TIME) && !game_over) {
			PlaySound(shoot_sound);
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

static void draw_and_update_bullets(float delta) {
	for (int i = 0; i < BULLETS_BUFFER_SIZE; i++) {
		if (bullet_buffer[i].visible) {
			//DrawCircle(bullet_buffer[i].pos.x, bullet_buffer[i].pos.y, BULLET_SIZE, WHITE);
			//DrawLineV(bullet_buffer[i].pos, , WHITE);
			DrawLineEx(
				bullet_buffer[i].pos,
				Vector2Add(bullet_buffer[i].pos, Vector2Scale((Vector2){bullet_buffer[i].dir.x, bullet_buffer[i].dir.y}, -50.0f)),
				3,
				bullet_buffer[i].enemy ? RED : BLUE
			);
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

			if (
				CheckCollisionCircles(bullet_buffer[i].pos, BULLET_SIZE, player_pos, PLAYER_SHIELD_RAD) &&
				player_shield_on &&
				bullet_buffer[i].enemy &&
				bullet_buffer[i].visible
			) {
				bullet_buffer[i].visible = false;
			}
		}
		
	}
}

static void draw_and_update_pickups(void) {
	Color health_background_color = GREEN;
	health_background_color.a = 100;
	Color shield_background_color = BLUE;
	shield_background_color.a = 100;

	for (int i = 0; i < ENEMIES_BUFFER_SIZE; i++) {
		if (health_pickups[i].shown) {
			DrawCircleV(health_pickups[i].pos, PICKUP_SIZE, health_background_color);
			DrawCircleV(health_pickups[i].pos, 5, GREEN);
			DrawRing(health_pickups[i].pos, PICKUP_SIZE, PICKUP_SIZE + 3, 0, 360, 100, GREEN);

			if (CheckCollisionCircles(health_pickups[i].pos, PICKUP_SIZE, player_pos, PLAYER_HIT_RADIUS)) {
				PlaySound(pickup_sound);
				health_pickups[i].shown = false;
				if (player_health != PLAYER_MAX_HEALTH) {
					player_health++;
				}
			}
		}

		if (shield_pickups[i].shown) {
			DrawCircleV(shield_pickups[i].pos, PICKUP_SIZE, shield_background_color);
			DrawCircleV(shield_pickups[i].pos, 5, BLUE);
			DrawRing(shield_pickups[i].pos, PICKUP_SIZE, PICKUP_SIZE + 3, 0, 360, 100, BLUE);

			if (CheckCollisionCircles(shield_pickups[i].pos, PICKUP_SIZE, player_pos, PLAYER_HIT_RADIUS)) {
				PlaySound(pickup_sound);
				player_shield_start_time = GetTime();
				player_shield_time = 5;
				player_shield_on = true;
				shield_pickups[i].shown = false;
			}
		}
	}
}

static void draw_and_update_astroids(float delta) {
	for (int i = 0; i < ASTROIDS_BUFFER_SIZE; i++) {
		astroid_buffer[i].pos = Vector2Add(astroid_buffer[i].pos, (Vector2){
			.x = -(astroid_buffer[i].dir.x * ASTROID_SPEED * delta),
			.y = -(astroid_buffer[i].dir.y * ASTROID_SPEED * delta)
		});

		if (Vector2Distance(astroid_buffer[i].pos, player_pos) < RENDER_DISTANCE) {
			//rf_draw_circle_lines(astroid_buffer[i].pos.x, astroid_buffer[i].pos.y, ASTROID_SIZE, RF_GRAY);
			//DrawRing(astroid_buffer[i].pos, ASTROID_SIZE, ASTROID_SIZE +3, 0,360, astroid_buffer[i].segments, GRAY);
			Color inside = GRAY;
			inside.a = 200;
			//DrawCircleSector(astroid_buffer[i].pos, ASTROID_SIZE, 0,360, astroid_buffer[i].segments, inside);
			//rf_draw_circle(astroid_buffer[i].pos.x, astroid_buffer[i].pos.y, ASTROID_SIZE, inside);

			DrawTexturePro(
				astroid_texture,
				(Rectangle){
					.x = 0, 
					.y = 0, 
					.width = astroid_texture.width,
					.height = astroid_texture.height
				},
				(Rectangle){
					.x = astroid_buffer[i].pos.x,
					.y = astroid_buffer[i].pos.y,
					.width = (ASTROID_SIZE*3),
					.height = (ASTROID_SIZE*3)
				},
				(Vector2){ (ASTROID_SIZE*3)/2, (ASTROID_SIZE*3)/2 },
				astroid_buffer[i].rotation,
				WHITE
			);

			if (show_debug_info) {
				DrawRing(astroid_buffer[i].pos, ASTROID_SIZE, ASTROID_SIZE +2, 0, 360, 100, GREEN);
			}
		}
	}
}

static void draw_window(Color color) {
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

static void draw_game_over(bool win) {
	draw_window(BLUE);
	const char *score = TextFormat("TIME: %.2f", game_over_time);
	const char *_previous_score = TextFormat("PREVIOUS TIME: %.2f", previous_score);
	char *over_text = "GAME OVER";
	char *win_text = "YOU WIN";

	char *current_text = win ? win_text : over_text; 

	int pos_x = (RENDER_WIDTH/2) - (MeasureTextEx(GetFontDefault(), current_text, 50, 20).x/2);
	int pos_y = (RENDER_HEIGHT/2) - 150;

	DrawText(current_text, pos_x, pos_y, 70, win ? GREEN : RED);

	if (win) {
		DrawText(score, (RENDER_WIDTH/2) - (MeasureText(score, 20)/2), (RENDER_HEIGHT/2)+210, 20, WHITE);
		DrawText(_previous_score, (RENDER_WIDTH/2) - (MeasureText(_previous_score, 20)/2), (RENDER_HEIGHT/2)+250, 20, WHITE);
	}

	update_and_draw_button(&buttons[0], virtual_mouse_pos, (Vector2){ RENDER_WIDTH, RENDER_HEIGHT });
	update_and_draw_button(&buttons[1], virtual_mouse_pos, (Vector2){ RENDER_WIDTH, RENDER_HEIGHT });
}

static void draw_game_pause() {
	draw_window(GREEN);

	char *text = "GAME PAUSED";
	char *hint = "Press [ESC] to Resume";

	int pos_x = (RENDER_WIDTH/2) - (MeasureTextEx(GetFontDefault(), text, 50, 20).x/2);
	int pos_y = (RENDER_HEIGHT/2) - 150;

	int hint_x = (RENDER_WIDTH/2) - (MeasureTextEx(GetFontDefault(), text, 20, 30).x/2);

	DrawText(text, pos_x, pos_y, 70, RED);
	DrawText(hint, hint_x, pos_y + 600, 40, BLUE);
	update_and_draw_button(&buttons[0], virtual_mouse_pos, (Vector2){ RENDER_WIDTH, RENDER_HEIGHT });
	update_and_draw_button(&buttons[1], virtual_mouse_pos, (Vector2){ RENDER_WIDTH, RENDER_HEIGHT });
}

static void draw_health(void) {
	Vector2 pos = { 10.0f, 20.0f };
	
	DrawRectangleLinesEx(
		(Rectangle){
			15,
			pos.y - 5,
			(40.0f)*PLAYER_MAX_HEALTH + 5,
			10.0f
		},
		2,
		GREEN
	);
	
	for (int i = 0; i < PLAYER_MAX_HEALTH; i++) {
		pos.x = (40.0f * i) + 20;
		DrawRectangleRec(
			(Rectangle){
				pos.x,
				pos.y,
				35.0f,
				10.0f,
			},
			BLACK
		);
		DrawRectangleLinesEx(
			(Rectangle){
				pos.x,
				pos.y,
				35.0f,
				10.0f,
			},
			1,
			GREEN
		);
		
	}

	pos.x = 10.0f;

	for (int i = 0; i < player_health; i++) {
		pos.x = (40.0f * i) + 20;
		DrawRectangleV(pos, (Vector2){ 35.0f, 10.0f }, GREEN);
	}
}

static void draw_enemies_left(void) {
	float left_offset = (RENDER_WIDTH/2) - ((ENEMIES_BUFFER_SIZE * 40.0f)/2);

	Vector2 pos = { 10.0f , 60.0f };

	DrawRectangleLinesEx(
		(Rectangle){
			(RENDER_WIDTH/2) - ((ENEMIES_BUFFER_SIZE * 40.0f)/2) - 5,
			pos.y - 5,
			(40.0f)*ENEMIES_BUFFER_SIZE,
			10.0f
		},
		2,
		RED
	);

	for (int i = 0; i < ENEMIES_BUFFER_SIZE; i++) {
		pos.x = (40.0f * i) + (RENDER_WIDTH/2) - ((ENEMIES_BUFFER_SIZE * 40.0f)/2);
		
		DrawRectangleRec(
			(Rectangle){
				pos.x,
				pos.y,
				30.0f,
				10.0f,
			},
			BLACK
		);

		DrawRectangleLinesEx(
			(Rectangle){
				pos.x,
				pos.y,
				30.0f,
				10.0f,
			},
			1,
			RED
		);
	}
	
	pos.x = 10.0f;

	for (int i = 0; i < enemies_left; i++) {
		pos.x = (40.0f * i) + left_offset;
		DrawRectangleV(pos, (Vector2){ 30.0f, 10.0f }, RED);
	}
}

static void draw_score(void) {
	const char *text = FormatText("TIME: %.2f", GetTime() - game_start_time);
	const char *previous_score_text = FormatText("PREVIOUS TIME: %.2f", previous_score);
	DrawText(text, RENDER_WIDTH-MeasureText("TIME: 000.00", 20)-10, 10, 20, WHITE);
	DrawText(previous_score_text, RENDER_WIDTH-MeasureText("PREVIOUS TIME: 000.00", 20)-10, 30, 20, WHITE);
}

static void retry(void) {
	reset_state();
}

static void exit_game(void) {
	game_exit = true;
}


static void handle_input(void) {
	if (IsKeyPressed(KEY_ESCAPE)) {
		if (!game_over)
			game_pause = !game_pause;
			PlaySound(click_sound);
	}

	if (IsKeyPressed(KEY_F1)) {
		show_debug_info = !show_debug_info;
	}
}

static void on_scene_load(void) {
	HideCursor();

	load_sounds();

	// Render texture initialization, used to hold the rendering result so we can easily resize it
	render_texture = LoadRenderTexture(RENDER_WIDTH, RENDER_HEIGHT);
	SetTextureFilter(render_texture.texture, FILTER_BILINEAR);

	player_texture = LoadTexture(ASSETS_PATH"image/ship_H.png");

	enemy_texture = LoadTexture(ASSETS_PATH"image/station_C.png");

	astroid_texture = LoadTexture(ASSETS_PATH"image/meteor_detailedLarge.png");

	cursor_texture = LoadTexture(ASSETS_PATH"image/crossair_white.png");

	reset_state();

	// Init Buttons
	buttons[0] = (button){
		.title = "Restart",
		.offset = 0,
		.border = false,
		.on_click = retry,
		.pressed = false
	};

	buttons[1] = (button){
		.title = "Exit Game",
		.offset = 90,
		.border = false,
		.on_click = exit_game,
		.pressed = false
	};
}

static void on_scene_update(void(*change_scene)(scene *scn), bool *should_exit, float delta) {
	//UpdateMusicStream(shoot_sound);

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
			Color background_color = YELLOW;
			background_color.a = 20; // I know I'll fix it later I promise

			DrawCircleSector((Vector2){0.0f, 0.0f}, GAME_BORDER_RADIUS + RENDER_DISTANCE, 0, 360, 150, background_color);
			DrawCircleSector((Vector2){0.0f, 0.0f}, GAME_BORDER_RADIUS, 0, 360, 150, ((Color){18, 18, 18, 255}));

			draw_and_update_bullets(delta);

			draw_and_update_player(delta);

			draw_and_update_astroids(delta);

			draw_and_update_enemies(delta);
			
			draw_and_update_pickups();

			// Draw game border
			DrawRing((Vector2){0.0f, 0.0f}, GAME_BORDER_RADIUS, GAME_BORDER_RADIUS+5, 0, 360, 150, GOLD);

			// draw_cool_circle(50, 20, 10, 60, RED);

			update_camera();

		EndMode2D();

		if (player_border_touch) {
			DrawText("Don't Cross the border", RENDER_WIDTH/2, RENDER_HEIGHT/2 - 70.0f, IN_GAME_FONT_SIZE, RED);
		}

		if (game_over) {
			char *game_over_message = "Man, You suck!";
			DrawText(game_over_message, (RENDER_WIDTH/2) - (MeasureText(game_over_message, IN_GAME_FONT_SIZE)/2), 20, IN_GAME_FONT_SIZE, RED);
			draw_game_over(false);
		} else {
			char *game_hint = "Destory enemies and avoid astroids";
			DrawText(game_hint, (RENDER_WIDTH/2) - (MeasureText(game_hint, IN_GAME_FONT_SIZE)/2), 20, IN_GAME_FONT_SIZE, RED);
		}

		if (!enemies_left) {
			if (!won) {
				game_over_time = GetTime() - game_start_time;
						
				if (game_over_time < previous_score) {
					SaveStorageValue(0, (int)(game_over_time*1000));
				}
			}
			won = true;
			draw_game_over(true);
		}
		
		if (!player_health && !game_over) {
			set_game_over();
		}

		if (!game_over && (GetTime() - player_shield_start_time) > player_shield_time) {
			player_shield_on = false;
		}

		draw_health();
		draw_enemies_left();
		draw_score();

		if (game_pause) {
			draw_game_pause();
		}

		// Draw cursor
		DrawTexturePro(
			cursor_texture,
			(Rectangle){
				.x = 0.0f,
				.y = 0.0f,
				.width = cursor_texture.width,
				.height = cursor_texture.height
			},
			(Rectangle) {
				virtual_mouse_pos.x,
				virtual_mouse_pos.y,
				25,
				25
			},
			(Vector2){ 25/2, 25/2 },
			0.0f,
			WHITE
		);

		if (show_debug_info) {
			const char *fps_output = TextFormat("FPS: %i", GetFPS());
			DrawText(fps_output, RENDER_WIDTH - MeasureText(fps_output, IN_GAME_FONT_SIZE) - 20 , RENDER_HEIGHT - 40, IN_GAME_FONT_SIZE, RED);
		}
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
		
    EndDrawing();

	handle_input();

}

static void on_scene_exit(void) {
	unload_sounds();
	UnloadRenderTexture(render_texture);
	UnloadTexture(player_texture);
	UnloadTexture(astroid_texture);
	UnloadTexture(enemy_texture);
}

scene game_scene = {
	.init_fn = on_scene_load,
	.update_fn = on_scene_update,
	.exit_fn = on_scene_exit,
};