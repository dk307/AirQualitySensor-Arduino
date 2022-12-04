#pragma once

#include <lvgl.h>

#include "sensor_id.h"

extern lv_obj_t *ui_main_screen;
extern lv_obj_t *ui_boot_message;

void ui_init();

void ui_inline_loop(uint64_t maxWait);
void ui_set_sensor_value(sensor_id_index id, uint16_t value, sensor_level level);

