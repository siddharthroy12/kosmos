#include "../scene.h"
#include "rayfork.h"
#include "sokol_time.h"
#include <stdio.h>

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

void update_camera(void) {
	camera.target = player_pos;
}

void draw_and_update_enemies(float delta) {
	for (int i = 0; i < ENEMIES_BUFFER_SIZE; i++) {
		if (enemy_buffer[i].health > 0) {

			// Update gun rotation
			rf_vec2 end_point = rf_vec2_add(enemy_buffer[i].pos, (rf_vec2){
				.x =  (rf_vec2_normalize(rf_vec2_sub(enemy_buffer[i].dir, enemy_buffer[i].pos)).x * 80),
				.y =  (rf_vec2_normalize(rf_vec2_sub(enemy_buffer[i].dir, enemy_buffer[i].pos)).y * 80),
			});

			// Move the gun and shoot when player comes closer
			if (rf_vec2_distance(player_pos, enemy_buffer[i].pos) < ENEMY_RADAR_RANGE) {
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

			if (rf_check_collision_circles(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, player_pos, RENDER_DISTANCE)) {
				rf_draw_ring(enemy_buffer[i].pos, ASTROID_SIZE, ASTROID_SIZE +3, 0,360, 5, RF_RED);
				rf_color inside = RF_RED;
				inside.a = 200;
				rf_draw_circle_sector(enemy_buffer[i].pos, ASTROID_SIZE, 0,360, 5, inside);

				// Draw rader range
				rf_draw_circle_sector_lines(enemy_buffer[i].pos, ENEMY_RADAR_RANGE, 0, 360, 100, RF_BLUE);

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

	// If cursor if away enough and player hasn't reached his max speed then accelerate or else deaccelerate
	if (accelerate && player_speed < PLAYER_MAX_SPEED && rf_vec2_distance(world_mouse_pos, player_pos) > 5.0f) {
		player_speed += PLAYER_ACCLERATION * delta;
	} else {
		player_speed = rf_lerp(player_speed, 0.0f, PLAYER_DEACCLERATION * delta);
	}

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
	if (shoot && (stm_ms(stm_since(shoot_start_time)) > 200.0)) {
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

	// Draw render distance border
	rf_draw_circle_lines(player_pos.x, player_pos.y, RENDER_DISTANCE, RF_RED);

	// Draw his radius
	//rf_draw_circle_lines(player_pos.x ,player_pos.y, PLAYER_HIT_RADIUS, RF_GREEN);
}

void draw_and_update_bullets(float delta) {
	for (int i = 0; i < BULLETS_BUFFER_SIZE; i++) {
		bullet_buffer[i].pos = rf_vec2_add(bullet_buffer[i].pos, (rf_vec2){
			.x = -(bullet_buffer[i].dir.x * BULLET_SPEED * delta),
			.y = -(bullet_buffer[i].dir.y * BULLET_SPEED * delta)
		});

		if (bullet_buffer[i].visible) {
			rf_draw_circle(bullet_buffer[i].pos.x, bullet_buffer[i].pos.y, BULLET_SIZE, RF_YELLOW);
		}

		for (int j = 0; j < ASTROIDS_BUFFER_SIZE; j++) {
			if (rf_check_collision_circles(bullet_buffer[i].pos, BULLET_SIZE, astroid_buffer[j].pos, ASTROID_SIZE)) {
				bullet_buffer[i].visible = false;
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

static void on_scene_load(void) {
	sapp_show_mouse(false);

	// Render texture initialization, used to hold the rendering result so we can easily resize it
	render_texture = rf_load_render_texture(RENDER_WIDTH, RENDER_HEIGHT);
	rf_set_texture_filter(render_texture.texture, RF_FILTER_BILINEAR);

	player_texture = rf_load_texture_from_file_ez(ASSETS_PATH"player.png");


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
			rf_draw_circle_sector_lines((rf_vec2){0.0f, 0.0f}, GAME_BORDER_RADIUS, 0, 360, 100, RF_GOLD);

			update_camera();

		rf_end_2d();

		if (game_over) {
			rf_draw_text("Game Over", 20, 20, 20, RF_RED);
		} else {
			rf_draw_text("Destory enemies and avoid astroids", 20, 20, 20, RF_RED);
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



const scene game_scene = {
	.init_fn = on_scene_load,
	.update_fn = on_scene_update,
	.exit_fn = on_scene_exit,
	.event_fn = on_event
};