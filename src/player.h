#pragma once

struct player_t {
	uint8_t radius;
	uint8_t x;
	double y;
	double accel_y;
	double vel_y;
};

void player_init();
void player_jump();
void player_layer_update_callback(Layer *me, GContext *ctx);
void calc_player();
void reset_player();
