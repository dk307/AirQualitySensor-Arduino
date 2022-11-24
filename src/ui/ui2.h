#pragma once

#include <lvgl.h>

extern lv_obj_t *ui_bootscreen;
extern lv_obj_t *ui_bootlogo;
extern lv_obj_t *ui_bootmessage;
void ui_event_mainscreen(lv_event_t *e);
extern lv_obj_t *ui_main_screen;
extern lv_obj_t *ui_settings_screen;

LV_IMG_DECLARE(ui_img_1508142627); // assets\icons8-wind-100.png

LV_FONT_DECLARE(ui_font_BigNumberCompact128);
LV_FONT_DECLARE(ui_font_BigNumberCompact96);

void ui_init();
void ui_set_aqi_value(uint16_t value);
