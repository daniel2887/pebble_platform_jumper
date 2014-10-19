#pragma once

uint16_t rand_range(uint16_t min, uint16_t max);
void game_over();

/* Game config */

#define ANIMATION_STEP_MS 50.0 /* 20fps */
#define GRAVITY -0.001 /* pix per ms^2 */
#define ACCEL_DEAD_ZONE 30.0 /* milli-G */

#define PLAYER_MAX_X_VEL 0.3 /* pix per ms */
#define PLAYER_MAX_X_ACCEL 707.1 /* milli-G */
#define PLAYER_RADIUS 10 /* pixels */
#define PLAYER_JUMP_VEL 0.3 /* pix per ms */
#define PLAYER_MAX_JUMPS 2
/* Player ideal max jump height is calculated by:
 * h = v^2 / (2*g) * num_jumps */
#define PLAYER_MAX_JUMP_HEIGHT_IDEAL \
	((PLAYER_JUMP_VEL * PLAYER_JUMP_VEL) / (2.0 * -1.0 * GRAVITY) * PLAYER_MAX_JUMPS)
/* Reduce player max jump height to this percentage
 * of ideal max jump height, since getting up to ideal
 * max jump height in reality is unlikely. */
#define PLAYER_MAX_JUMP_HEIGHT (0.9 * PLAYER_MAX_JUMP_HEIGHT_IDEAL)

#define PLATFORM_THICKNESS 3 /* pixels */
#define PLATFORM_MIN_LEN (PLAYER_RADIUS * 2)
#define PLATFORM_MAX_LEN (PLAYER_RADIUS * 6)
#define PLATFORM_SPEED 0.036 /* pix per ms */
#define PLATFORM_SPEED_INCR_PER_LEVEL (PLATFORM_SPEED * 0.2)
#define PLATFORM_SPAWN_RATE_MIN 800 /* ms */
#define PLATFORM_SPAWN_RATE_MAX 500 /* ms */

#define GAME_INCR_LEVEL_EVERY_N_PTS 10
