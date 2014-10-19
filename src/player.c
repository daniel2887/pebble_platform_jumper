#include <pebble.h>
#include "platform_jumper.h"
#include "player.h"
#include "platforms.h"

/* TODO: Wrap these up in a game-wide context
 * and pass context handle to helper functions*/
extern struct platform_t *platform_list;
extern struct player_t player;
extern GRect window_frame;

void player_init()
{
	player.radius = PLAYER_RADIUS;
	player.x = player.radius * 2;
	player.y = 0;
	player_layer_dirty = true;
}

void player_jump()
{
	player.vel_y = PLAYER_JUMP_VEL;
}

void player_layer_update_callback(Layer *me, GContext *ctx)
{
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_circle(ctx, GPoint(player.x, player.y), player.radius);
}

void calc_player()
{
	static double last_y;
	struct platform_t *platform_itr = platform_list;

	/* Upwards movement is negative in terms of
	 * display coordinates */
	player.y -= player.vel_y * ANIMATION_STEP_MS;

	/* Check if player collided with top of window */
	if (player.y < player.radius) {
		player.y = player.radius;
		player.vel_y = 0;
	}

	/* Check if player collided with a platform */
	for (platform_itr = platform_list;
		platform_itr != NULL;
		platform_itr = platform_itr->next) {
		GRect *data = &platform_itr->data;

		/* Collision if:
		 * 	- Platform is located under the player
		 * 	- Bottom tip of the player located within the platform
		 * 	- Ball is falling or is stationary
		 * 	*/
		if (player.x >= data->origin.x &&
			player.x <= (data->origin.x + data->size.w) &&
			last_y + player.radius <= data->origin.y &&
			player.y + player.radius >= data->origin.y &&
			player.vel_y <= 0) {
			player.y = data->origin.y - player.radius;
			player.vel_y = 0;
			}
	}

	/* Check if player dropped below window */
	if (player.y - player.radius > window_frame.size.h) {
		/* Keep player hidden below bottom of window */
		player.y = window_frame.size.h + player.radius;
		player.vel_y = 0;
		game_over();
	}

	player.vel_y += GRAVITY * ANIMATION_STEP_MS;

	player_layer_dirty = (player.y != last_y);

	last_y = player.y;
}

void reset_player()
{
	player_init();
}
