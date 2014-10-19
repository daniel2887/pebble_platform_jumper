#include <pebble.h>
#include "platform_jumper.h"
#include "player.h"
#include "platforms.h"

static Window *window;
static Layer *player_layer;
static Layer *platforms_layer;
static Layer *game_over_layer;
bool game_is_over = false;

struct platform_t *platform_list = NULL;
struct player_t player;
GRect window_frame;

static AppTimer *timer;

bool reset_game_req = false;

uint16_t rand_range(uint16_t min, uint16_t max)
{
	return (max - min) * (double)rand() / (double) RAND_MAX + min;
}

static void reset_game()
{
	game_is_over = false;
	layer_set_hidden(game_over_layer, true);

	reset_platforms();
	reset_player();
}

static void game_over_layer_update_callback(Layer *me, GContext *ctx)
{
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx,
		"gg\n(Up to respawn)",
		fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
		GRect(0, window_frame.size.h / 3, window_frame.size.w, window_frame.size.h),
		GTextOverflowModeWordWrap,
		GTextAlignmentCenter,
		NULL);
}

void game_over()
{
	if (game_is_over)
		return;

	layer_set_hidden(game_over_layer, false);

	game_is_over = true;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
	/* Ignore player jumps if game is lost */
	if (game_is_over)
		return;

	player_jump();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
	reset_game_req = true;
}

static void click_config_provider(void *context)
{
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect frame = window_frame = layer_get_frame(window_layer);

	platforms_layer = layer_create(frame);
	layer_set_update_proc(platforms_layer, platforms_layer_update_callback);
	layer_add_child(window_layer, platforms_layer);
	
	player_layer = layer_create(frame);
	layer_set_update_proc(player_layer, player_layer_update_callback);
	layer_add_child(window_layer, player_layer);

	game_over_layer = layer_create(frame);
	layer_set_update_proc(game_over_layer, game_over_layer_update_callback);
	layer_add_child(window_layer, game_over_layer);
	layer_set_hidden(game_over_layer, true);
	layer_mark_dirty(game_over_layer);

	player_init();
	platforms_init();
}

static void window_unload(Window *window)
{
	layer_destroy(player_layer);
	layer_destroy(platforms_layer);
	layer_destroy(game_over_layer);
}

static void timer_callback(void *data)
{

	if (reset_game_req) {
		reset_game();
		reset_game_req = false;
	}

	calc_platforms();

	layer_mark_dirty(platforms_layer);

	calc_player();
	if (player_layer_dirty)
		layer_mark_dirty(player_layer);

	timer = app_timer_register(ANIMATION_STEP_MS, timer_callback, NULL);
}

static void init(void)
{
	window = window_create();
	window_set_click_config_provider(window, click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
	  .load = window_load,
	  .unload = window_unload,
	});
	const bool animated = true;
	window_stack_push(window, animated);

	timer = app_timer_register(ANIMATION_STEP_MS, timer_callback, NULL);
}

static void deinit(void)
{
	window_destroy(window);
}

int main(void)
{
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
}
