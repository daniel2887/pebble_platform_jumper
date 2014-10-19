#pragma once

struct player_t {
	uint8_t radius;
	uint8_t x;
	double y;
	double last_y;
	double accel_y;
	double vel_y;
	int16_t platform_num; /* Player last landed on this platform */
	uint8_t jumps_taken; /* Jumps taken since last landing */
};

void player_init();
void player_jump();
void player_layer_update_callback(Layer *me, GContext *ctx);
void calc_player();
void reset_player();
