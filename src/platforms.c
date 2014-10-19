#include <pebble.h>
#include "platform_jumper.h"
#include "platforms.h"
#include "player.h"

/* TODO: Wrap these up in a game-wide context
 * and pass context handle to helper functions*/
extern struct platform_t *platform_list;
extern struct player_t player;
extern GRect window_frame;

void platforms_init()
{
	struct platform_t *platform = malloc(sizeof(struct platform_t));
	if (!platform) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Can't allocate memory for platform list\n");
		return;
	}

	platform->data.origin.x = 0;
	platform->data.origin.y = rand_range(player.radius * 3, window_frame.size.h);
	platform->data.size.h = PLATFORM_THICKNESS;
	platform->data.size.w = window_frame.size.w;
	platform->next = NULL;
	platform->prev = NULL;
	platform->platform_num = 0;

	platform_list = platform;

	/* TODO: just for debug */
	//platform_list->data.origin.x = 0;
}

void platforms_layer_update_callback(Layer *me, GContext *ctx)
{
	struct platform_t *platform_itr = platform_list;
	
	for (platform_itr = platform_list;
		platform_itr != NULL;
		platform_itr = platform_itr->next) {

		graphics_context_set_fill_color(ctx, GColorBlack);
		graphics_fill_rect (ctx, platform_itr->data, 0, 0);
	}
}

void platform_spawn()
{
	struct platform_t *tail;
	struct platform_t *platform = malloc(sizeof(struct platform_t));
	static uint16_t platform_num = 1;
	int16_t platform_spawn_y_min, platform_spawn_y_max;
	if (!platform) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Can't allocate memory for new platform\n");
		return;
	}

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


	/* Make sure that the new platform is reachable
	 * by the player */
	if(platform->prev) {
		platform_spawn_y_min = platform->prev->data.origin.y - PLAYER_MAX_JUMP_HEIGHT;
		if (platform_spawn_y_min < player.radius * 3)
			platform_spawn_y_min = player.radius * 3;

		platform_spawn_y_max = platform->prev->data.origin.y + PLAYER_MAX_JUMP_HEIGHT;
		if (platform_spawn_y_max > window_frame.size.h)
			platform_spawn_y_max = window_frame.size.h;
	} else {
		platform_spawn_y_min = player.radius * 3;
		platform_spawn_y_max = window_frame.size.h;
	}

	platform->data.origin.x = window_frame.size.w + 1; /* Just outside window */
	platform->data.origin.y = rand_range(platform_spawn_y_min, platform_spawn_y_max);
	platform->data.size.h = PLATFORM_THICKNESS;
	platform->data.size.w = rand_range(PLATFORM_MIN_LEN, PLATFORM_MAX_LEN);
	platform->next = NULL;
	platform->prev = NULL;
	platform->platform_num = platform_num++;

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

void calc_platforms()
{
	static int16_t time_to_spawn_platform = 0;
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

	/* Time to spawn a new platform? */
	if (!platforms_pending()) {
		time_to_spawn_platform -= ANIMATION_STEP_MS;
		if (time_to_spawn_platform <= 0) {
			platform_spawn();
			time_to_spawn_platform = rand_range(PLATFORM_SPAWN_RATE_MIN, PLATFORM_SPAWN_RATE_MAX);
		}
	}
}

void reset_platforms()
{
	struct platform_t *platform_itr = platform_list;

	/* Remove all platforms */
	while (platform_itr) {
		struct platform_t *temp = platform_itr;
		platform_itr = platform_itr->next;
		free(temp);
	}

	platform_list = NULL;

	/* Respawn initial platform */
	platforms_init();
}
