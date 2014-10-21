#pragma once

#include "platforms.h"

struct player_t {
	uint8_t radius;

	int16_t x;
	int16_t last_x;
	double vel_x;

	uint8_t y;
	uint8_t last_y;
	double vel_y;

	int16_t platform_num; /* Player last landed on this platform */
	uint8_t jumps_taken; /* Jumps taken since last landing */
};

void player_init(struct player_t *player);
void player_jump(struct player_t *player);
void calc_player(struct player_t *player, struct platform_t *platform_list);
void reset_player(struct player_t *player);
