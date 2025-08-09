
#include "settings_view.h"

#include <stdio.h>
#include <stdlib.h>

#include "HomeTUI/home_tui.h"
#include "app_context.h"

struct settings_view {
	struct AppContext * ctx;
	struct ui * ui;
	int is_editing_text;
};

static int in_settings_window = 1; //TODO: Get rid of static

void onCancellSettings(struct ui_button *b) { in_settings_window = 0; }
void onSaveSettings(struct ui_button *b) { in_settings_window = 0; };


struct settings_view *create_settings_view(struct AppContext *ctx, AppAction save_settings) {
	struct settings_view *view = (struct settings_view *)malloc(sizeof(struct settings_view));

	if(!view)
		return NULL;

	view->ctx = ctx;

	struct ui *ui = ui_create();
	view->ui = ui;

	if(!ui) {
		fprintf(stderr, "Settings view: failed to create ui\n");
		free(view);
		return NULL;
	}

	ui_add_label(ui, 3, 5, 21, 2, "Stopped on app start");
	ui_add_checkbox(ui, 26, 5, ctx->settings.stopped_on_app_start, NULL);

	char buf[100];
	sprintf(buf, "%d", ctx->settings.stop_after_min);

	ui_add_label(ui, 3, 9, 21, 2, "Stop after: ");
	ui_add_textbox(ui, 26, 9, 21, 2, buf, NULL);

	sprintf(buf, "%d", ctx->settings.min_seconds_to_save);
	ui_add_label(ui, 3, 13, 21, 2, "Min save time, sec");
	ui_add_textbox(ui, 26, 13, 21, 2, buf, NULL);

	ui_add_button(ui, 3, 17, 7, 2, "Cancel", onCancellSettings);
	ui_add_button(ui, 15, 17, 5, 2, "Save", onSaveSettings);

	return view;
}

int handle_input_settings_view(struct settings_view *view, int input_key) {
	ui_process_input(view->ui, input_key);
	if(input_key != -1/*IDLE_INPUT*/){ //TODO: Take this def from HomeTUI
		render_settings_view(view);
	}

	if(in_settings_window == 0) { //TODO: This is temporary Exit, call onSave/onCancel callback
		view->ctx->view = TIMER_VIEW;
		in_settings_window = 1;
	}

	return 0;
}

void render_settings_view(struct settings_view *view)
{
	ui_render(view->ui);
	render_clear();
	render_text(3, 1, "Settings");
	render_text(3, 2, "--------");
	ui_render(view->ui);
	render_update();
}

void dispose_settings_view(struct settings_view *v)
{
	if (v)
		free(v);
}
