#ifndef _TTIMER_HELP_VIEW_H_
#define _TTIMER_HELP_VIEW_H_

#include "HomeTUI/home_tui.h"

void render_help()
{
	int cols, lines;
	get_window_size(&cols, &lines);

	render_clear();

	render_text(5, 1, "TTimer");
	render_text(5, 2, "------");
	render_text(5, 3, "'Space' - pause/resume timer");
	render_text(5, 3, "'q'     - quit timer and save your time");
	render_text(5, 4, "'Esc'   - back to timer");
	render_text(5, 5, "'g'     - show time graph");
	render_text(5, 6, "'s'     - show settings");
	render_text(5, 7, "'h'     - show this help");
	render_text(5, 9, "In graph view");
	render_text(5, 10, "-------------");
	render_text(5, 11, "'h'     - switch 1 day backwards");
	render_text(5, 12, "'l'     - switch 1 day forward");
	render_text(5, 13, "'r'     - reset to today");
	render_ftext(5, 14, "Window size: %d %d", lines, cols);

	render_update();
}

#endif
