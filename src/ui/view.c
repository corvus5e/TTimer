#include "view.h"
#include <stdlib.h>

struct view {
	RenderFunc render;
	ProcessInputFunc process_input;
	DisposeFunc dispose;
	void *owner;
};

struct view *view_create(RenderFunc render,
			 ProcessInputFunc process_input,
			 DisposeFunc dispose, void *owner) {
	struct view *v = (struct view*)malloc(sizeof(struct view));
	if(v) {
		v->render = render;
		v->process_input = process_input;
		v->dispose = dispose;
		v->owner = owner;
		return v;
	}

	return NULL;
}

void *get_owner(const struct view* v) {
	return v->owner;
}

void view_render(const struct view *v) {
	if(v && v->render) {
		v->render(v);
	}
}

int view_process_input(struct view *v, int key) {
	if(v &&  v->process_input) {
		return v->process_input(v, key);
	}
	return 0;
}

void view_dispose(struct view *v) {
	if(v && v->dispose) {
		v->dispose(v);
	}
}


