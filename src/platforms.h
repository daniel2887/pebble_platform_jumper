#pragma once

struct platform_t {
	GRect data;
	struct platform_t *next;
	struct platform_t *prev;
	uint16_t platform_num;
};

void platforms_init(struct platform_t **platform_list);
void calc_platforms(struct platform_t **platform_list);
void platform_spawn(struct platform_t **platform_list);
void reset_platforms(struct platform_t **platform_list);
