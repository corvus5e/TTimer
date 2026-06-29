
#include "settings_view.h"

#include <stdio.h>
#include <stdlib.h>

#include "HomeTUI/home_tui.h"
#include "app_context.h"
#include "ui/view.h"

#define INPUT_BUF_LEN 50

struct settings_ui {
	struct ui *ui_ctx;

	struct ui_checkbox * stopped_on_start_check_box;
	struct ui_textbox * stop_after_textbox;
	struct ui_textbox * min_time_save_textbox;
	struct ui_textbox * idle_time_pause_textbox;
	struct ui_checkbox * save_on_term;

	int	   stopped_on_app_start_buf;
	char	   stop_after_min_buf[INPUT_BUF_LEN];
	char	   min_seconds_to_save_buf[INPUT_BUF_LEN];
	char	   idle_pause_time_buf[INPUT_BUF_LEN];
};

struct settings_view {
	struct view * view;
	struct AppContext * ctx;
        struct settings_ui ui;
	SaveSettings save_settings_func;
	GetSettings get_settings_func;
};

int handle_input_settings_view(struct view *, int input_key);

void render_settings_view(const struct view *);

void dispose_settings_view(struct view *);

static void on_save_settings(void * arg)
{
	struct settings_view *sv = (struct settings_view *)arg;

	if(!sv) {
		return;
	}

	struct AppSettings new_settings;
	new_settings.stopped_on_app_start = ui_is_checked(sv->ui.stopped_on_start_check_box);
	new_settings.stop_after_min	  = atoi(ui_get_text(UI_BOX(sv->ui.stop_after_textbox)));
	new_settings.min_seconds_to_save  = atoi(ui_get_text(UI_BOX(sv->ui.min_time_save_textbox)));
	new_settings.save_on_term_signal  = ui_is_checked(sv->ui.save_on_term);
	new_settings.idle_pause_time	  = atoi(ui_get_text(UI_BOX(sv->ui.idle_time_pause_textbox)));

	if(sv->ctx && sv->save_settings_func) {
		sv->save_settings_func(sv->ctx, new_settings);
	}
};

struct view *create_settings_view(struct AppContext *ctx,
				  SaveSettings save_settings,
				  GetSettings get_settings)
{
	struct settings_view *sv = (struct settings_view *)malloc(sizeof(struct settings_view));

	if (!sv)
		return NULL;

	sv->view = view_create(render_settings_view, handle_input_settings_view, dispose_settings_view, sv);

	if(!sv->view) {
		return NULL;
	}

	sv->ctx = ctx;
	sv->save_settings_func = save_settings;
	sv->get_settings_func = get_settings;

	struct ui *ui_ctx = ui_create();
	sv->ui.ui_ctx = ui_ctx;

	if (!ui_ctx) {
		fprintf(stderr, "Settings view: failed to create ui\n");
		free(sv);
		return NULL;
	}

	ui_add_box(ui_ctx, 3, 5, 21, CONST_STR_ARG("Stopped on app start"));
	sv->ui.stopped_on_start_check_box = ui_add_checkbox(
	    ui_ctx, 26, 5, ctx->settings.stopped_on_app_start, nullptr, nullptr);

	sprintf(sv->ui.stop_after_min_buf, "%d", ctx->settings.stop_after_min);

	ui_add_box(ui_ctx, 3, 9, 21, CONST_STR_ARG("Stop after, min"));
	sv->ui.stop_after_textbox = ui_add_textbox(ui_ctx, 26, 9, 21, sv->ui.stop_after_min_buf,
						   sizeof(sv->ui.stop_after_min_buf), nullptr, nullptr);

	sprintf(sv->ui.min_seconds_to_save_buf, "%d", ctx->settings.min_seconds_to_save);
	ui_add_box(ui_ctx, 3, 13, 21, CONST_STR_ARG("Min save time, sec"));
	sv->ui.min_time_save_textbox = ui_add_textbox(ui_ctx, 26, 13, 21, sv->ui.min_seconds_to_save_buf,
						      sizeof(sv->ui.min_seconds_to_save_buf), nullptr, nullptr);

	ui_add_box(ui_ctx, 35, 5, 21, CONST_STR_ARG("Save on TERM, sec"));
	sv->ui.save_on_term = ui_add_checkbox(ui_ctx, 58, 5, ctx->settings.save_on_term_signal, nullptr, nullptr);

	ui_add_box(ui_ctx, 35, 9, 21, CONST_STR_ARG("Pause on idle, sec"));
	sprintf(sv->ui.idle_pause_time_buf, "%d", ctx->settings.idle_pause_time);
	sv->ui.idle_time_pause_textbox = ui_add_textbox(ui_ctx, 58, 9, 21, sv->ui.idle_pause_time_buf, sizeof(sv->ui.idle_pause_time_buf), nullptr, nullptr);

	ui_add_button(ui_ctx, 3, 17, 5, CONST_STR_ARG("Save"), on_save_settings, sv);

	return sv->view;
}

int handle_input_settings_view(struct view *v, int input_key) {
	struct settings_view* view = get_owner(v);

	if(!view) {
		fprintf(stderr, "Owner is NULL");
		return IGNORED;
	}

	return ui_process_input(view->ui.ui_ctx, input_key);
}

void render_settings_view(const struct view *v)
{
	struct settings_view* view = get_owner(v);

	if(!view) {
		fprintf(stderr, "Owner is NULL");
		return;
	}

	//view->get_settings_func(&view->ctx->settings);
	//TODO: copy data from settings to ui;
	render_clear();
	render_text(3, 1, "Settings");
	render_text(3, 2, "--------");
	ui_render(view->ui.ui_ctx);
	render_update();
}

void dispose_settings_view(struct view *v)
{
	struct settings_view* view = get_owner(v);

	if(!view) {
		fprintf(stderr, "Owner is NULL");
		return;
	}

	free(view);
}
