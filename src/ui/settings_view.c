
#include "settings_view.h"

#include <stdio.h>
#include <stdlib.h>

#include "HomeTUI/home_tui.h"
#include "app_context.h"

struct settings_view {
	struct AppContext * ctx;
	struct ui * ui;
	struct ui_checkbox * stopped_on_start_check_box;
	struct ui_textbox * stop_after_textbox;
	struct ui_textbox * min_time_save_textbox;
	struct ui_checkbox * save_on_term;
	SaveSettings save_settings_func;
	GetSettings get_settings_func;
};

static void on_save_settings(struct ui_button *b, void * arg)
{
	struct settings_view *sv = (struct settings_view *)arg;

	if(!sv) {
		return;
	}

	struct AppSettings new_settings;
	new_settings.stopped_on_app_start = ui_is_checked(sv->stopped_on_start_check_box);
	new_settings.stop_after_min = atoi(ui_get_text(UI_BOX(sv->stop_after_textbox)));
	new_settings.min_seconds_to_save = atoi(ui_get_text(UI_BOX(sv->min_time_save_textbox)));
	new_settings.save_on_term_signal = ui_is_checked(sv->save_on_term);

	if(sv->ctx && sv->save_settings_func) {
		sv->save_settings_func(sv->ctx, new_settings);
	}
};

struct settings_view *create_settings_view(struct AppContext *ctx,
					   SaveSettings save_settings,
					   GetSettings get_settings)
{
	struct settings_view *view = (struct settings_view *)malloc(sizeof(struct settings_view));

	if (!view)
		return NULL;

	view->ctx = ctx;
	view->save_settings_func = save_settings;
	view->get_settings_func = get_settings;

	struct ui *ui = ui_create();
	view->ui = ui;

	if (!ui) {
		fprintf(stderr, "Settings view: failed to create ui\n");
		free(view);
		return NULL;
	}

	ui_add_box(ui, 3, 5, 21, 2, "Stopped on app start");
	view->stopped_on_start_check_box = ui_add_checkbox(
	    ui, 26, 5, ctx->settings.stopped_on_app_start, NULL);

	char buf[100];
	sprintf(buf, "%d", ctx->settings.stop_after_min);

	ui_add_box(ui, 3, 9, 21, 2, "Stop after, min");
	view->stop_after_textbox =  ui_add_textbox(ui, 26, 9, 21, 2, buf, NULL);

	sprintf(buf, "%d", ctx->settings.min_seconds_to_save);
	ui_add_box(ui, 3, 13, 21, 2, "Min save time, sec");
	view->min_time_save_textbox = ui_add_textbox(ui, 26, 13, 21, 2, buf, NULL);

	ui_add_box(ui, 35, 5, 21, 2, "Save on TERM, sec");
	view->save_on_term = ui_add_checkbox(ui, 58, 5, ctx->settings.save_on_term_signal, NULL);

	ui_add_button(ui, 3, 17, 5, 2, "Save", on_save_settings, view);

	return view;
}

int handle_input_settings_view(struct settings_view *view, int input_key) {
	return ui_process_input(view->ui, input_key);
}

void render_settings_view(struct settings_view *view)
{
	//view->get_settings_func(&view->ctx->settings);
	//TODO: copy data from settings to ui;
	render_clear();
	render_text(3, 1, "Settings");
	render_text(3, 2, "--------");
	ui_render(view->ui);
	render_update();
}

void dispose_settings_view(struct settings_view *view)
{
	free(view);
}
