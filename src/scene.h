#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>

// Fix the ASSETS_PATH not defined error in vscode
#ifndef ASSETS_PATH
#define ASSETS_PATH
#endif

typedef struct scene scene;

struct scene {
	void (*init_fn)(void);
	void (*update_fn)(void(*change_scene)(scene *scn), bool *should_exit, float delta);
	void (*exit_fn)(void);
};

scene load_scene(scene *scn);
void switch_scene(scene *scn1, scene *scn2);

#endif