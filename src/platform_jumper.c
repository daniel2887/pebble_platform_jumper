#include <pebble.h>

#define ANIMATION_STEP_MS 50.0 /* 20fps */
#define BALL_RADIUS 10
#define BALL_JUMP_VEL 0.3 /* pix per ms */
#define GRAVITY -0.001 /* pix per ms^2 */
#define PLATFORM_THICKNESS 3
#define PLATFORM_MIN_LEN (BALL_RADIUS * 2)
#define PLATFORM_MAX_LEN (BALL_RADIUS * 6)
#define PLATFORM_SPEED 0.036
#define PLATFORM_SPAWN_RATE_MIN 800 /* ms */
#define PLATFORM_SPAWN_RATE_MAX 500 /* ms */

static Window *window;
//static TextLayer *text_layer;
static Layer *ball_layer;
bool ball_layer_dirty;
static Layer *platforms_layer;
static Layer *game_over_layer;
bool game_is_over = false;
static GRect window_frame;

static AppTimer *timer;

struct ball_t {
	uint8_t radius;
	uint8_t x;
	double y;
	double accel_y;
	double vel_y;
} ball;

struct platform_t {
	GRect data;
	struct platform_t *next;
	struct platform_t *prev;
	uint16_t platform_num;
};

struct platform_t *platform_list = 0;
bool reset_game_req = false;

static uint16_t rand_range(uint16_t min, uint16_t max)
{
	return (max - min) * (double)rand() / (double) RAND_MAX + min;
}

static void ball_init()
{
	ball.radius = BALL_RADIUS;
	ball.x = ball.radius * 2;
	ball.y = 0;
	ball_layer_dirty = true;
}

static void platforms_init()
{
	struct platform_t *platform = malloc(sizeof(struct platform_t));
	if (!platform) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Can't allocate memory for platform list\n");
		return;
	}

	platform->data.origin.x = 0;
	platform->data.origin.y = rand_range(ball.radius * 3, window_frame.size.h);
	platform->data.size.h = PLATFORM_THICKNESS;
	platform->data.size.w = window_frame.size.w;
	platform->next = NULL;
	platform->prev = NULL;
	platform->platform_num = 0;

	platform_list = platform;

	/* TODO: just for debug */
	//platform_list->data.origin.x = 0;
}

static void reset_game()
{
	struct platform_t *platform_itr = platform_list;

	game_is_over = false;
	layer_set_hidden(game_over_layer, true);

	/* Remove all platforms */
	while (platform_itr) {
		struct platform_t *temp = platform_itr;
		platform_itr = platform_itr->next;
		free(temp);
	}

	platform_list = NULL;

	/* Respawn initial platform */
	platforms_init();

	/* Reset ball location */
	ball_init();
}

static void game_over_layer_update_callback(Layer *me, GContext *ctx) {
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx,
		"gg :(\n(Up to respawn)",
		fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
		GRect(0, window_frame.size.h / 3, window_frame.size.w, window_frame.size.h),
		GTextOverflowModeWordWrap,
		GTextAlignmentCenter,
		NULL);
}

static void game_over()
{
	if (game_is_over)
		return;

	layer_set_hidden(game_over_layer, false);

	game_is_over = true;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	/* Don't allow ball jump if game is lost */
	if (game_is_over)
		return;

	ball.vel_y = BALL_JUMP_VEL;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	reset_game_req = true;
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
}

static void ball_layer_update_callback(Layer *me, GContext *ctx) {
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, GPoint(ball.x, ball.y), ball.radius);
}

static void platforms_layer_update_callback(Layer *me, GContext *ctx) {
	struct platform_t *platform_itr = platform_list;
	
	for (platform_itr = platform_list;
		platform_itr != NULL;
		platform_itr = platform_itr->next) {

		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect (ctx, platform_itr->data, 0, 0);
	}
}

static void platform_spawn()
{
	struct platform_t *tail;
	struct platform_t *platform = malloc(sizeof(struct platform_t));
	static uint16_t platform_num = 1;
	if (!platform) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Can't allocate memory for new platform\n");
		return;
	}

	platform->data.origin.x = window_frame.size.w + 1; /* Just outside window */
	platform->data.origin.y = rand_range(ball.radius * 3, window_frame.size.h);
	platform->data.size.h = PLATFORM_THICKNESS;
	platform->data.size.w = rand_range(PLATFORM_MIN_LEN, PLATFORM_MAX_LEN);
	platform->next = NULL;
	platform->prev = NULL;
	platform->platform_num = platform_num++;

	/* Append platform to list */
	tail = platform_list;
	if (!tail)
		platform_list = platform;
	else {
		while (tail->next)
			tail = tail->next;
		tail->next = platform;
		platform->prev = tail;
	}
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect frame = window_frame = layer_get_frame(window_layer);

	platforms_layer = layer_create(frame);
	layer_set_update_proc(platforms_layer, platforms_layer_update_callback);
	layer_add_child(window_layer, platforms_layer);
	
	ball_layer = layer_create(frame);
	layer_set_update_proc(ball_layer, ball_layer_update_callback);
	layer_add_child(window_layer, ball_layer);

	game_over_layer = layer_create(frame);
	layer_set_update_proc(game_over_layer, game_over_layer_update_callback);
	layer_add_child(window_layer, game_over_layer);
	layer_set_hidden(game_over_layer, true);
	layer_mark_dirty(game_over_layer);

	ball_init();
	platforms_init();
}

static void window_unload(Window *window) {
	layer_destroy(ball_layer);
	layer_destroy(platforms_layer);
	layer_destroy(game_over_layer);
}

static void calc_platforms()
{
	struct platform_t *platform_itr = platform_list;

	/* Remove any platforms that are no longer visible */
	platform_itr = platform_list;
	while (platform_itr) {
		struct platform_t *temp = platform_itr->next;

		/* Advance all platforms to the left (round speed) */
		platform_itr->data.origin.x -= (int16_t)(PLATFORM_SPEED * ANIMATION_STEP_MS + 0.5);

		if ((platform_itr->data.origin.x + platform_itr->data.size.w) < 0) {
			if (platform_itr->prev)
				platform_itr->prev->next = platform_itr->next;

			if (platform_itr->next)
				platform_itr->next->prev = platform_itr->prev;

			if (platform_itr == platform_list)
				platform_list = platform_list->next;

			free(platform_itr);
		}

		platform_itr = temp;
	}
}

static void calc_ball()
{
	static double last_y;
	struct platform_t *platform_itr = platform_list;

	/* Upwards movement is negative in terms of
	 * display coordinates */
	ball.y -= ball.vel_y * ANIMATION_STEP_MS;

	/* Check if ball collided with top of window */
	if (ball.y < ball.radius) {
		ball.y = ball.radius;
		ball.vel_y = 0;
	}

	/* Check if ball collided with a platform */
	for (platform_itr = platform_list;
		platform_itr != NULL;
		platform_itr = platform_itr->next) {
		GRect *data = &platform_itr->data;

		/* Collision if:
		 * 	- Platform is located under the ball
		 * 	- Bottom tip of the ball located within the platform
		 * 	- Ball is falling or is stationary
		 * 	*/
		if (ball.x >= data->origin.x &&
			ball.x <= (data->origin.x + data->size.w) &&
			last_y + ball.radius <= data->origin.y &&
			ball.y + ball.radius >= data->origin.y &&
			ball.vel_y <= 0) {
			ball.y = data->origin.y - ball.radius;
			ball.vel_y = 0;
			}
	}

	/* Check if ball dropped below window */
	if (ball.y - ball.radius > window_frame.size.h) {
		/* Keep ball hidden below bottom of window */
		ball.y = window_frame.size.h + ball.radius;
		ball.vel_y = 0;
		game_over();
	}

	ball.vel_y += GRAVITY * ANIMATION_STEP_MS;

	ball_layer_dirty = (ball.y != last_y);

	last_y = ball.y;
}

/* Returns true if there are any platforms
 * partially or fully hidden to the right of the
 * screen
 */
static bool platforms_pending()
{
	struct platform_t *platform_itr = platform_list;
	for (platform_itr = platform_list;
		platform_itr != NULL;
		platform_itr = platform_itr->next) {
		GRect *data = &platform_itr->data;
		if ((data->origin.x + data->size.w) > window_frame.size.w)
			return true;
	}
	return false;
}

static void timer_callback(void *data) {
	static int16_t time_to_spawn_platform = 0;

	if (reset_game_req) {
		reset_game();
		reset_game_req = false;
	}

	calc_platforms();

	if (!platforms_pending()) {
		time_to_spawn_platform -= ANIMATION_STEP_MS;
		if (time_to_spawn_platform <= 0) {
			platform_spawn();
			time_to_spawn_platform = rand_range(PLATFORM_SPAWN_RATE_MIN, PLATFORM_SPAWN_RATE_MAX);
		}
	}

	layer_mark_dirty(platforms_layer);

	calc_ball();
	if (ball_layer_dirty)
		layer_mark_dirty(ball_layer);

	timer = app_timer_register(ANIMATION_STEP_MS, timer_callback, NULL);
}

static void init(void) {
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

static void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
}
