
#include "settings_view.h"

#include <stdio.h>
#include <stdlib.h>

#include "HomeTUI/home_tui.h"
#include "app_context.h"

struct settings_view {
	struct AppContext * ctx;
	struct ui * ui;
	AppAction on_save_settings;
};

static void onSaveSettings(struct ui_button *b, void * arg)
{
	struct settings_view *sv = (struct settings_view *)arg;
	if(sv && sv->ctx && sv->on_save_settings) {
		sv->on_save_settings(sv->ctx);
	}
};

struct settings_view *create_settings_view(struct AppContext *ctx, AppAction save_settings) {
	struct settings_view *view = (struct settings_view *)malloc(sizeof(struct settings_view));

	if(!view)
		return NULL;

	view->ctx = ctx;
	view->on_save_settings = save_settings;

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

	ui_add_button(ui, 3, 17, 5, 2, "Save", onSaveSettings, view);

	return view;
}

int handle_input_settings_view(struct settings_view *view, int input_key) {
	return ui_process_input(view->ui, input_key);
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
