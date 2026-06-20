#ifndef _TTIMER_VIEW_H_
#define _TTIMER_VIEW_H_

struct view;
#define VIEW(obj) ((struct view*)(obj))

typedef void (*RenderFunc)(const struct view*);
typedef int (*ProcessInputFunc)(struct view*, int key);
typedef void (*DisposeFunc)(struct view*);

struct view *view_create(RenderFunc, ProcessInputFunc, DisposeFunc, void *owner);
void *get_owner(const struct view*);
void view_render(const struct view*);
int view_process_input(struct view*, int);
void view_dispose(struct view*);

#endif
