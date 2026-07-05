#include <ui/help_view.h>

#include <stdlib.h>

#include <HomeTUI/home_tui.h>
#include <ui/view.h>

void render_help_view(const struct view *v)
{
	int cols, lines;
	get_window_size(&cols, &lines);

	render_clear();

        int y = 1;

	render_text(5, y++, "TTimer");
	render_text(5, y++, "------");
	render_text(5, y++, "'Space' - pause/resume timer");
	render_text(5, y++, "'r'     - reset timer (writes time)");
	render_text(5, y++, "'q'     - quit timer and save your time");
	render_text(5, y++, "'Esc'   - back to timer");
	render_text(5, y++, "'g'     - show time graph");
	render_text(5, y++, "'s'     - show settings");
	render_text(5, y++, "'h'     - show this help");
	render_text(5, y++, "In graph view");
	render_text(5, y++, "-------------");
	render_text(5, y++, "'h'     - switch 1 day backwards");
	render_text(5, y++, "'l'     - switch 1 day forward");
	render_text(5, y++, "'r'     - reset to today");
	render_ftext(5, y++, "Window size: %d %d", lines, cols);

	render_update();
}

void dispose_help_view(struct view *v) { free(v); }

int handle_input_help_view(struct view *v, int key) { return IGNORED;}

struct view *create_help_view() {
	return view_create(render_help_view, handle_input_help_view, dispose_help_view, NULL);
}
