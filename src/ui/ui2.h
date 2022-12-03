#pragma once

#include <lvgl.h>

extern lv_obj_t *ui_main_screen;
extern lv_obj_t *ui_boot_message;

void ui_init();
void ui_set_aqi_value(uint16_t value);
void ui_inline_loop(uint64_t maxWait);

