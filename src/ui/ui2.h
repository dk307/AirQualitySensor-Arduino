#pragma once

#include <lvgl.h>

#include "sensor_id.h"
#include <task_wrapper.h>

class ui
{
public:
    static ui instance;
    void init();
    void update_boot_message(const std::string &message);
    void set_sensor_value(sensor_id_index id, uint16_t value, sensor_level level);
    void set_main_screen();

private:
    ui() = default;

    // boot screen
    lv_obj_t *boot_message;
    lv_obj_t *bootscreen;
    lv_obj_t *bootlogo;

    // main screen
    lv_obj_t *main_screen;

    typedef struct
    {
        lv_obj_t *panel{nullptr};
        lv_obj_t *label{nullptr};
    } panel_and_label;

    std::array<panel_and_label, total_sensors> main_screen_panel_and_label;

    // settings screen
    lv_obj_t *settings_screen;
    lv_obj_t *settings_screen_tab_information_table;
    lv_obj_t *settings_screen_tab_settings_brightness_slider;

    const lv_font_t *font_large = &lv_font_montserrat_24;
    const lv_font_t *font_normal = &lv_font_montserrat_14;
    const lv_font_t *font_extra_large_number = &lv_font_montserrat_20;

    // loaded from sd card
    lv_font_t *font_montserrat_light_numbers_96;
    lv_font_t *font_montserrat_light_numbers_112;

    lv_style_t style_text_muted;
    lv_style_t style_title;
    lv_style_t style_label_default;

    std::unique_ptr<task_wrapper> information_refresh_task;

    void inline_loop(uint64_t maxWait);
    void set_label_panel_color(lv_obj_t *panel, uint64_t level);
    void event_mainscreen(lv_event_t *e);
    void bootscreen_screen_init(void);
    panel_and_label main_screen_create_panel(const char *label_text,
                                             lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w, lv_coord_t h);
    void main_screen_screen_init(void);
    void load_information();
    void settings_screen_events_callback(lv_event_t *e);
    void settings_screen_screen_init(void);
    void settings_screen_tab_settings_brightness_slider_event_cb(lv_event_t *e);
    void load_from_sd_card();
};