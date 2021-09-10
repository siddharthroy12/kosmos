#include "../scene.h"
#include "rayfork.h"
#include "sokol_time.h"
#include <stdio.h>
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
	rf_vec2 pos;
	rf_vec2 dir;
	bool visible;
	bool enemy;
} bullet;

typedef struct astroid {
	rf_vec2 pos;
	rf_vec2 dir;
	int segments;
} astroid;

typedef struct enemy {
	rf_vec2 pos;
	rf_vec2 dir;
	int health;
} enemy;

astroid astroid_buffer[ASTROIDS_BUFFER_SIZE] = { 0 };

bullet bullet_buffer[BULLETS_BUFFER_SIZE] = { 0 };
int current_bullet_buffer = 0;

enemy enemy_buffer[ENEMIES_BUFFER_SIZE] = { 0 };

// Mouse
rf_vec2 mouse_pos = { 0 };
rf_vec2 virtual_mouse_pos = { 0 };

// Actions
bool accelerate = false;
bool shoot = false;

// Player
rf_texture2d player_texture = { 0 };
rf_vec2 player_pos = { 0.0f, 0.0f };
rf_vec2 player_dir = { 0.0f, 0.0f };
float player_speed = 0.0f;
bool game_over = false;
bool game_pause = false;

// Timer
uint64_t shoot_start_time;

// Render Texture
rf_render_texture2d render_texture;

// Camera
rf_camera2d camera = {
    .offset = (rf_vec2) { (float)RENDER_WIDTH/2 ,(float) RENDER_HEIGHT/2 },
    .target = (rf_vec2) { 0.0f ,0.0f },
    .rotation = 0.0f,
    .zoom = 1.0f,
};

// Buttons
button buttons[2];

static const char* text_format(const char* format, ...) {
    // We create an array of buffers so strings don't expire until 4 invocations
    static char buffers[4][100] = { 0 };
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, 100);   // Clear buffer before using

    va_list args;
    va_start(args, format);
    vsnprintf(currentBuffer, 100, format, args);
    va_end(args);

    index += 1;     // Move to next buffer for next function call
    if (index >= 4) index = 0;

    return currentBuffer;
}

rf_vec2 clamp_value(rf_vec2 value, rf_vec2 min, rf_vec2 max) {
	rf_vec2 result = value;
	result.x = (result.x > max.x)? max.x : result.x;
    result.x = (result.x < min.x)? min.x : result.x;
    result.y = (result.y > max.y)? max.y : result.y;
    result.y = (result.y < min.y)? min.y : result.y;
    return result;
}

void reset_state(void) {
	current_bullet_buffer = 0;
	player_pos = (rf_vec2){ 0.0f, 0.0f };
	player_dir = (rf_vec2){ 0.0f, 0.0f };
	player_speed = 0.0f;
	game_over = false;
	game_pause = false;
	shoot = false;

	// Init bullets
	for (int i = 0; i < BULLETS_BUFFER_SIZE; i++) {
		bullet_buffer[i] = (bullet){
			.dir = (rf_vec2){ 0.0f, 0.0f },
			.pos = (rf_vec2){ 0.0f, 0.0f },
			.visible = false,
			.enemy = false
		};
	}

	// Init Astroids
	for (int i = 0; i < ASTROIDS_BUFFER_SIZE; i++) {
		astroid_buffer[i] = (astroid){
			.dir = (rf_vec2){ 0.0f, 0.0f },
			.pos = (rf_vec2){ (float)rand_range(-GAME_BORDER_RADIUS, GAME_BORDER_RADIUS), (float)rand_range(-GAME_BORDER_RADIUS, GAME_BORDER_RADIUS) },
			.segments = rand_range(5, 10)
		};
	}

	// Init Enemies
	for (int i = 0; i < ENEMIES_BUFFER_SIZE; i++) {
		enemy_buffer[i] = (enemy){
			.pos = (rf_vec2){ (float)rand_range(-GAME_BORDER_RADIUS, GAME_BORDER_RADIUS), (float)rand_range(-GAME_BORDER_RADIUS, GAME_BORDER_RADIUS) },
			.dir = (rf_vec2){ 0.0f, 0.0f },
			.health = ENEMY_MAX_HEALTH
		};
	}
}

void update_camera(void) {
	camera.target = player_pos;
}

void draw_and_update_enemies(float delta) {
	rf_vec2 end_point = { 0 };
	for (int i = 0; i < ENEMIES_BUFFER_SIZE; i++) {
		if (enemy_buffer[i].health > 0) {
			
			// Update gun rotation
			end_point = rf_vec2_add(enemy_buffer[i].pos, (rf_vec2){
				.x =  (rf_vec2_normalize(rf_vec2_sub(enemy_buffer[i].dir, enemy_buffer[i].pos)).x * 80),
				.y =  (rf_vec2_normalize(rf_vec2_sub(enemy_buffer[i].dir, enemy_buffer[i].pos)).y * 80),
			});

			if (!game_pause) {
				
				// Move the gun and shoot when player comes closer
				if (rf_vec2_distance(player_pos, enemy_buffer[i].pos) < ENEMY_RADAR_RANGE && !game_over) {
					enemy_buffer[i].dir.x = rf_lerp(enemy_buffer[i].dir.x, player_pos.x, delta * 0.002);
					enemy_buffer[i].dir.y = rf_lerp(enemy_buffer[i].dir.y, player_pos.y, delta * 0.002);

					bullet_buffer[current_bullet_buffer].enemy = true;
					bullet_buffer[current_bullet_buffer].visible = true;
					bullet_buffer[current_bullet_buffer].pos = enemy_buffer[i].pos;
					bullet_buffer[current_bullet_buffer].dir = rf_vec2_normalize(rf_vec2_sub(bullet_buffer[current_bullet_buffer].pos, end_point));
					
					if (current_bullet_buffer == BULLETS_BUFFER_SIZE - 1) {
						current_bullet_buffer = 0;
					} else {
						current_bullet_buffer++;
					}
				} 

				// Check player collision
				if (rf_check_collision_circles(enemy_buffer[i].pos, ENEMY_HIT_RANGE, player_pos, PLAYER_HIT_RADIUS)) {
					game_over = true;
				}

				// Check Player Bullet collision
				for (int j = 0; j < BULLETS_BUFFER_SIZE; j++) {
					if (rf_check_collision_circles(enemy_buffer[i].pos, ENEMY_HIT_RANGE, bullet_buffer[j].pos, BULLET_SIZE)) {
						if (bullet_buffer[j].visible && !bullet_buffer[j].enemy) {
							enemy_buffer[i].health--;
							bullet_buffer[j].visible = false;
						}
						
					}
				}
			}

			if (rf_check_collision_circles(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, player_pos, RENDER_DISTANCE)) {
				rf_draw_ring(enemy_buffer[i].pos, ASTROID_SIZE, ASTROID_SIZE +3, 0,360, 5, RF_RED);
				rf_color inside = RF_RED;
				inside.a = 200;
				rf_draw_circle_sector(enemy_buffer[i].pos, ASTROID_SIZE, 0,360, 5, inside);

				// Draw rader range
				inside = RF_BLUE;
				inside.a = 5;
				rf_draw_circle_sector(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, 0, 360, 100, inside);
				rf_draw_ring(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, ENEMY_RADAR_RANGE+3, 0, 360, 100, RF_SKYBLUE);

				// Draw gun
				rf_draw_line_ex(enemy_buffer[i].pos, end_point , 10, RF_LIME);

				// Draw Health
				rf_draw_text(text_format("HEALTH: %i", enemy_buffer[i].health), enemy_buffer[i].pos.x - 30, enemy_buffer[i].pos.y - 80, 20, RF_YELLOW);
			}
		}
	}
}

void draw_and_update_player(float delta) {
	rf_vec2 world_mouse_pos = rf_get_screen_to_world2d(virtual_mouse_pos, camera);
	float rotation = rf_vec2_angle(player_pos, world_mouse_pos) + 90.0f;

	// Draw player sprite
	rf_draw_texture_region(
		player_texture,
		(rf_rec){
			.x = 0,
			.y = 0,
			.width = player_texture.width,
			.height = player_texture.height
		},
		(rf_rec){
			.x = player_pos.x,
			.y = player_pos.y,
			.width = 20,
			.height = 20,
		},
		(rf_vec2){ 10, 10 },
		rotation,
		RF_WHITE
	);

	if (!game_pause) {
		// If cursor if away enough and player hasn't reached his max speed then accelerate or else deaccelerate
		if (accelerate && player_speed < PLAYER_MAX_SPEED && rf_vec2_distance(world_mouse_pos, player_pos) > 5.0f && !game_over) {
			player_speed += PLAYER_ACCLERATION * delta;
		} else {
			player_speed = rf_lerp(player_speed, 0.0f, PLAYER_DEACCLERATION * delta);
		}

		// Get the direction from player to cursor
		player_dir = rf_vec2_normalize(rf_vec2_sub(player_pos, world_mouse_pos));

		// Move the player towards direction
		if (rf_vec2_distance(world_mouse_pos, player_pos) > 5.0f) {
			player_pos = rf_vec2_add(player_pos, (rf_vec2){
				.x = -(player_dir.x * player_speed * delta),
				.y = -(player_dir.y * player_speed * delta)
			});
		}

		// Don't let player cross the border
		float distance = rf_vec2_distance((rf_vec2){ 0.0f, 0.0f}, player_pos);

		if (distance > GAME_BORDER_RADIUS) {
			rf_draw_text("Don't Cross the border", player_pos.x, player_pos.y - 40.0f, 20, RF_RED);
			rf_vec2 origin_to_player = rf_vec2_normalize(rf_vec2_sub(player_pos, (rf_vec2){0.0f,0.0f}));
			player_pos = rf_vec2_scale(origin_to_player, GAME_BORDER_RADIUS);
		}

		// Check astroid collision
		for (int i = 0; i < ASTROIDS_BUFFER_SIZE; i++) {
			if (rf_check_collision_circles(astroid_buffer[i].pos, ASTROID_SIZE, player_pos, PLAYER_HIT_RADIUS)) {
				game_over = true;
			}
		}

		// Check bullet collision
		for (int i = 0; i < BULLETS_BUFFER_SIZE; i++) {
			if (rf_check_collision_circles(bullet_buffer[i].pos, BULLET_SIZE, player_pos, PLAYER_HIT_RADIUS)) {
				if (bullet_buffer[i].visible && bullet_buffer[i].enemy)
					game_over = true;
			}
		}

		// Handle shoot logic
		if (shoot && (stm_ms(stm_since(shoot_start_time)) > 200.0) && !game_over) {
			bullet_buffer[current_bullet_buffer].enemy = false;
			bullet_buffer[current_bullet_buffer].visible = true;
			bullet_buffer[current_bullet_buffer].pos = player_pos;
			bullet_buffer[current_bullet_buffer].dir = rf_vec2_normalize(rf_vec2_sub(player_pos, world_mouse_pos));
			if (current_bullet_buffer == BULLETS_BUFFER_SIZE - 1) {
				current_bullet_buffer = 0;
			} else {
				current_bullet_buffer++;
			}

			shoot_start_time = stm_now();
		}
	}


	// Draw render distance border
	rf_draw_circle_lines(player_pos.x, player_pos.y, RENDER_DISTANCE, RF_RED);

	// Draw his radius
	//rf_draw_circle_lines(player_pos.x ,player_pos.y, PLAYER_HIT_RADIUS, RF_GREEN);
}

void draw_and_update_bullets(float delta) {
	for (int i = 0; i < BULLETS_BUFFER_SIZE; i++) {
		if (bullet_buffer[i].visible) {
			rf_draw_circle(bullet_buffer[i].pos.x, bullet_buffer[i].pos.y, BULLET_SIZE, RF_YELLOW);
		}

		if (!game_pause) {
			bullet_buffer[i].pos = rf_vec2_add(bullet_buffer[i].pos, (rf_vec2){
				.x = -(bullet_buffer[i].dir.x * BULLET_SPEED * delta),
				.y = -(bullet_buffer[i].dir.y * BULLET_SPEED * delta)
			});

			for (int j = 0; j < ASTROIDS_BUFFER_SIZE; j++) {
				if (rf_check_collision_circles(bullet_buffer[i].pos, BULLET_SIZE, astroid_buffer[j].pos, ASTROID_SIZE)) {
					bullet_buffer[i].visible = false;
				}
			}
		}
		
	}
}

void draw_and_update_astroids(float delta) {
	for (int i = 0; i < ASTROIDS_BUFFER_SIZE; i++) {
		astroid_buffer[i].pos = rf_vec2_add(astroid_buffer[i].pos, (rf_vec2){
			.x = -(astroid_buffer[i].dir.x * ASTROID_SPEED * delta),
			.y = -(astroid_buffer[i].dir.y * ASTROID_SPEED * delta)
		});

		if (rf_vec2_distance(astroid_buffer[i].pos, player_pos) < RENDER_DISTANCE) {
			//rf_draw_circle_lines(astroid_buffer[i].pos.x, astroid_buffer[i].pos.y, ASTROID_SIZE, RF_GRAY);
			rf_draw_ring(astroid_buffer[i].pos, ASTROID_SIZE, ASTROID_SIZE +3, 0,360, astroid_buffer[i].segments, RF_GRAY);
			rf_color inside = RF_GRAY;
			inside.a = 200;
			rf_draw_circle_sector(astroid_buffer[i].pos, ASTROID_SIZE, 0,360, astroid_buffer[i].segments, inside);
			//rf_draw_circle(astroid_buffer[i].pos.x, astroid_buffer[i].pos.y, ASTROID_SIZE, inside);
		}
	}
}

void draw_window(rf_color color) {
	rf_rec rec = {
		.x = (RENDER_WIDTH/2) - (OVERLAY_WINDOW_WIDTH/2),
		.y = (RENDER_HEIGHT/2) - (OVERLAY_WINDOW_HEIGHT/2),
		.width = OVERLAY_WINDOW_WIDTH,
		.height = OVERLAY_WINDOW_HEIGHT,
	};

	rf_color background_color = color;
	background_color.a = 50;

	rf_draw_rectangle_rec(rec, (rf_color){18, 18, 18, 255});
	rf_draw_rectangle_rec(rec, background_color);
	rf_draw_rectangle_outline(rec, 3, color);
}

void draw_game_over(void) {
	draw_window(RF_BLUE);

	char *text = "GAME OVER";
	int pos_x = (RENDER_WIDTH/2) - (rf_measure_text(rf_get_default_font(), text, 50, 5).width/2);
	int pos_y = (RENDER_HEIGHT/2) - 100;

	rf_draw_text(text, pos_x, pos_y, 50, RF_RED);

	update_and_draw_button(&buttons[0], virtual_mouse_pos, shoot, (rf_vec2){ RENDER_WIDTH, RENDER_HEIGHT });
	update_and_draw_button(&buttons[1], virtual_mouse_pos, shoot, (rf_vec2){ RENDER_WIDTH, RENDER_HEIGHT });
}

void draw_game_pause() {
	draw_window(RF_GREEN);

	char *text = "GAME PAUSED";
	char *hint = "Press [ESC] to Resume";

	int pos_x = (RENDER_WIDTH/2) - (rf_measure_text(rf_get_default_font(), text, 50, 5).width/2);
	int pos_y = (RENDER_HEIGHT/2) - 100;

	int hint_x = (RENDER_WIDTH/2) - (rf_measure_text(rf_get_default_font(), text, 20, 10).width/2);

	rf_draw_text(text, pos_x, pos_y, 50, RF_RED);
	rf_draw_text(hint, hint_x, pos_y + 400, 20, RF_BLUE);
	update_and_draw_button(&buttons[0], virtual_mouse_pos, shoot, (rf_vec2){ RENDER_WIDTH, RENDER_HEIGHT });
	update_and_draw_button(&buttons[1], virtual_mouse_pos, shoot, (rf_vec2){ RENDER_WIDTH, RENDER_HEIGHT });
}

static void retry(void) {
	reset_state();
}

static void exit_game(void) {
	sapp_quit();
}

static void on_scene_load(void) {
	sapp_show_mouse(false);

	// Render texture initialization, used to hold the rendering result so we can easily resize it
	render_texture = rf_load_render_texture(RENDER_WIDTH, RENDER_HEIGHT);
	rf_set_texture_filter(render_texture.texture, RF_FILTER_BILINEAR);

	player_texture = rf_load_texture_from_file_ez(ASSETS_PATH"player.png");

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

static void on_scene_update(void(*change_scene)(scene *scn), float delta) {

	// Compute required framebuffer scaling
    float scale = min((float)sapp_width()/RENDER_WIDTH, (float)sapp_height()/RENDER_HEIGHT);

	// Update virtual mouse (clamped mouse value behind game screen)
    virtual_mouse_pos.x = (mouse_pos.x - (sapp_width() - (RENDER_WIDTH*scale))*0.5f)/scale;
    virtual_mouse_pos.y = (mouse_pos.y - (sapp_height() - (RENDER_HEIGHT*scale))*0.5f)/scale;
    virtual_mouse_pos = clamp_value(virtual_mouse_pos, (rf_vec2){ 0, 0 }, (rf_vec2){ (float)RENDER_WIDTH, (float)RENDER_HEIGHT });

	rf_begin_render_to_texture(render_texture);
		rf_clear((rf_color){18, 18, 18, 255});

		rf_begin_2d(camera);
			draw_and_update_player(delta);

			draw_and_update_bullets(delta);

			draw_and_update_astroids(delta);

			draw_and_update_enemies(delta);

			// Draw game border
			rf_draw_ring((rf_vec2){0.0f, 0.0f}, GAME_BORDER_RADIUS, GAME_BORDER_RADIUS+5, 0, 360, 150, RF_GOLD);

			update_camera();

		rf_end_2d();

		if (game_over) {
			rf_draw_text("Man, You suck!", 20, RENDER_HEIGHT - 40, 20, RF_RED);
			draw_game_over();
		} else {
			rf_draw_text("Destory enemies and avoid astroids", 20, RENDER_HEIGHT - 40, 20, RF_RED);
		}

		if (game_pause) {
			draw_game_pause();
		}

		// Draw cursor
		rf_draw_circle(virtual_mouse_pos.x, virtual_mouse_pos.y, 2, RF_WHITE);
	rf_end_render_to_texture();

	rf_begin();
        rf_clear(RF_BLACK);
		// Draw render texture to screen, properly scaled
		rf_draw_texture_region(
			render_texture.texture,
			(rf_rec){ 0.0f, 0.0f, (float)render_texture.texture.width, (float)-render_texture.texture.height },
			(rf_rec){
				(sapp_width() - ((float)RENDER_WIDTH*scale))*0.5f,
				(sapp_height() - ((float)RENDER_HEIGHT*scale))*0.5f,
				(float)RENDER_WIDTH*scale,
				(float)RENDER_HEIGHT*scale
			},
			(rf_vec2){ 0, 0 },
			0.0f,
			RF_WHITE
		);

    rf_end();

}

void handle_key_event(const sapp_event* event, const bool value) {
	switch (event->key_code) {
		case SAPP_KEYCODE_SPACE:
			accelerate = value;
			break;
		case SAPP_KEYCODE_W:
			shoot = value;
			break;
		default:
			break;
	}
}

void handle_mouse_button_event(const sapp_event *event, const bool value) {
	switch (event->mouse_button) {
		case SAPP_MOUSEBUTTON_LEFT:
			shoot = value;
			break;
		case SAPP_MOUSEBUTTON_RIGHT:
			accelerate = value;
			break;
		default:
			break;
	}
}

static void on_event(const sapp_event *event) {
	switch (event->type) {
		case SAPP_EVENTTYPE_MOUSE_MOVE:
			mouse_pos.x = event->mouse_x;
			mouse_pos.y = event->mouse_y;
			break;
		case SAPP_EVENTTYPE_MOUSE_DOWN:
			handle_mouse_button_event(event, true);
			break;
		case SAPP_EVENTTYPE_MOUSE_UP:
			handle_mouse_button_event(event, false);
			break;
		case SAPP_EVENTTYPE_KEY_DOWN:
			switch (event->key_code) {
				case SAPP_KEYCODE_ESCAPE:
					if (!game_over)
						game_pause = !game_pause;
				break;
			}
			handle_key_event(event, true);
			break;
		case SAPP_EVENTTYPE_KEY_UP:
			handle_key_event(event, false);
			break;
		default:
			break;
	}
}

static void on_scene_exit(void) {
	printf("game Scene exit\n");
}

scene game_scene = {
	.init_fn = on_scene_load,
	.update_fn = on_scene_update,
	.exit_fn = on_scene_exit,
	.event_fn = on_event
};