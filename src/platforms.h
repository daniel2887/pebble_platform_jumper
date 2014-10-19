#pragma once

struct platform_t {
	GRect data;
	struct platform_t *next;
	struct platform_t *prev;
	uint16_t platform_num;
};

void platforms_init();
void platforms_layer_update_callback(Layer *me, GContext *ctx);
void calc_platforms();
void platform_spawn();
void reset_platforms();
