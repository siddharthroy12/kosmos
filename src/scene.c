#include "scene.h"

scene load_scene(scene *scn) {
	scn->init_fn();
	return *scn;
}

void switch_scene(scene *scn1, scene *scn2) {
	scn1->exit_fn();
	scn2->init_fn();
	*scn1 = *scn2;
}