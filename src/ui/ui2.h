#pragma once

#include <lvgl.h>

extern lv_obj_t *ui_main_screen;
extern lv_obj_t *ui_boot_message;


//LV_FONT_DECLARE(ui_font_BigNumberCompact128);
//LV_FONT_DECLARE(ui_font_BigNumberCompact96);

void ui_init();
void ui_set_aqi_value(uint16_t value);
