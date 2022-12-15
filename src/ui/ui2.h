#pragma once

#include <lvgl.h>
#include <WString.h>

#include "sensor_id.h"
#include "sensor.h"
#include "ui_interface.h"
#include <task_wrapper.h>

class ui
{
public:
    ui(ui_interface &ui_interface_) : ui_interface_instance(ui_interface_)
    {
    }
    void init();
    void update_boot_message(const String &message);
    void set_sensor_value(sensor_id_index id, sensor_value::value_type value);
    void set_main_screen();

private:
    ui_interface &ui_interface_instance;

    // boot screen
    lv_obj_t *boot_message;
    lv_obj_t *boot_screen;
    lv_obj_t *boot_logo;

    // main screen
    lv_obj_t *main_screen;

    typedef struct
    {
        lv_obj_t *panel{nullptr};
        lv_obj_t *label{nullptr};
    } panel_and_label;

    std::array<panel_and_label, total_sensors> main_screen_panel_and_label;

    // sensor detail screen
    lv_obj_t *sensor_detail_screen;
    lv_obj_t *sensor_detail_screen_top_label;
    lv_obj_t *sensor_detail_screen_current_value_label;
    lv_obj_t *sensor_detail_screen_current_value_unit_label;

    // settings screen
    lv_obj_t *settings_screen;
    lv_obj_t *settings_screen_tab_information_table;
    lv_obj_t *settings_screen_tab_settings_brightness_slider;

    const lv_font_t *font_large = &lv_font_montserrat_24;
    const lv_font_t *font_normal = &lv_font_montserrat_14;

    // loaded from sd card
    lv_font_t *font_montserrat_light_numbers_48;
    lv_font_t *font_montserrat_light_numbers_96;
    lv_font_t *font_montserrat_light_numbers_112;
    lv_font_t *font_montserrat_bold_numbers_48;

    lv_style_t style_text_muted;
    lv_style_t style_title;
    lv_style_t style_label_default;

    std::unique_ptr<task_wrapper> information_refresh_task;

    void inline_loop(uint64_t maxWait);
    void set_label_panel_color(lv_obj_t *panel, uint64_t level);
    void event_main_screen(lv_event_t *e);
    void bootscreen_screen_init(void);
    panel_and_label main_screen_create_big_panel(sensor_id_index index,
                                                 lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w, lv_coord_t h);
    panel_and_label main_screen_create_small_panel(sensor_id_index index,
                                                   lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w, lv_coord_t h);
    panel_and_label main_screen_create_temperature_panel(sensor_id_index index, lv_coord_t x_ofs, lv_coord_t y_ofs);
    panel_and_label main_screen_create_humidity_panel(sensor_id_index index, lv_coord_t x_ofs, lv_coord_t y_ofs);
    void boot_screen_screen_init(void);
    void main_screen_screen_init(void);
    void sensor_detail_screen_init(void);
    void load_information();
    void settings_screen_events_callback(lv_event_t *e);
    void settings_screen_screen_init(void);
    void settings_screen_tab_settings_brightness_slider_event_cb(lv_event_t *e);
    void load_from_sd_card();
    void show_sensor_detail_screen(sensor_id_index index);
    void detail_screen_current_values(sensor_id_index index, sensor_value::value_type value);

    static void set_padding_zero(lv_obj_t *obj);
    void create_close_button_to_main_screen(lv_obj_t *parent);
    static lv_obj_t *create_sensor_detail_screen_label(lv_obj_t *parent, const lv_font_t *font);

    void add_panel_callback_event(lv_obj_t *panel, sensor_id_index index);
    struct _lv_event_dsc_t *add_event_callback(lv_obj_t *obj, std::function<void(lv_event_t *)> ftn, lv_event_code_t filter);
};
