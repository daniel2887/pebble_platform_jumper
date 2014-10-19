#pragma once

uint16_t rand_range(uint16_t min, uint16_t max);
void game_over();

/* Game config */
#define ANIMATION_STEP_MS 50.0 /* 20fps */
#define PLAYER_RADIUS 10
#define PLAYER_JUMP_VEL 0.3 /* pix per ms */
#define GRAVITY -0.001 /* pix per ms^2 */
#define PLATFORM_THICKNESS 3
#define PLATFORM_MIN_LEN (PLAYER_RADIUS * 2)
#define PLATFORM_MAX_LEN (PLAYER_RADIUS * 6)
#define PLATFORM_SPEED 0.036
#define PLATFORM_SPAWN_RATE_MIN 800 /* ms */
#define PLATFORM_SPAWN_RATE_MAX 500 /* ms */
