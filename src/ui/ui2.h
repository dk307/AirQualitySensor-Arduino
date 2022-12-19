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
    void set_sensor_value(sensor_id_index id, const std::optional<sensor_value::value_type> &value);
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
    lv_obj_t *sensor_detail_screen_top_label_units;
    lv_obj_t *sensor_detail_screen_chart;
    lv_chart_series_t *sensor_detail_screen_chart_series;
    std::vector<sensor_value::value_type, psram::allocator<sensor_value::value_type>> sensor_detail_screen_chart_series_data;
    std::optional<time_t> sensor_detail_screen_chart_series_data_time;

    std::array<panel_and_label, 4> sensor_detail_screen_label_and_unit_labels;
    const size_t label_and_unit_label_current_index = 0;
    const size_t label_and_unit_label_average_index = 1;
    const size_t label_and_unit_label_min_index = 2;
    const size_t label_and_unit_label_max_index = 3;

    // settings screen
    lv_obj_t *settings_screen;
    lv_obj_t *settings_screen_tab_settings_kb;
    lv_obj_t *settings_screen_tab_information_table;
    lv_obj_t *host_name_text_area;
    lv_obj_t *ntp_server_text_area;
    lv_obj_t *ntp_server_refresh_interval_label_spinbox;
    lv_obj_t *settings_screen_tab_settings_brightness_slider;

    std::unique_ptr<task_wrapper> information_refresh_task;

    const lv_font_t *font_large = &lv_font_montserrat_24;

    // fonts loaded from sd card - bpp4
    lv_font_t *font_montserrat_light_numbers_112;  // 0x20,0,1,2,3,4,5,6,7,8,9,-
    lv_font_t *font_montserrat_regular_numbers_48; // 0x20,0,1,2,3,4,5,6,7,8,9,-
    lv_font_t *font_montserrat_regular_numbers_40; // 0x20,0,1,2,3,4,5,6,7,8,9,-
    lv_font_t *font_montserrat_medium_48;          // 0x20-0x7F,0,1,2,3,4,5,6,7,8,9,F,µ,g,/,m,³,°,F,⁒,p,-
    lv_font_t *font_montserrat_medium_units_18;    // 0x20,F,µ,g,/,m,³,°,F,⁒,p,-
    lv_font_t *font_montserrat_medium_14;          // 0x20-0x7F,0,1,2,3,4,5,6,7,8,9,F,µ,g,/,m,³,°,F,⁒,p,-

    

    void inline_loop(uint64_t maxWait);
    static void set_label_panel_color(lv_obj_t *panel, uint8_t level);
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
    void detail_screen_current_values(sensor_id_index index, const std::optional<sensor_value::value_type> &value);
    static void set_padding_zero(lv_obj_t *obj);
    void create_close_button_to_main_screen(lv_obj_t *parent);
    static lv_obj_t *create_sensor_detail_screen_label(lv_obj_t *parent, const lv_font_t *font,
                                                       lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs,
                                                       lv_color_t color);
    static lv_coord_t get_label_height(lv_obj_t *label);
    panel_and_label create_detail_screen_panel(const char *label_text,
                                               lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs,
                                               lv_coord_t w, lv_coord_t h);

    static void set_value_in_panel(const panel_and_label &pair, sensor_id_index index, const std::optional<sensor_value::value_type> &value);
    static void set_default_value_in_panel(const panel_and_label &pair);
    void add_panel_callback_event(lv_obj_t *panel, sensor_id_index index);
    struct _lv_event_dsc_t *add_event_callback(lv_obj_t *obj, std::function<void(lv_event_t *)> ftn, lv_event_code_t filter = LV_EVENT_ALL);
    void chart_draw_event_cb(lv_event_t *e);
    void settings_screen_screen_host_name_event_cb(lv_event_t *e);
    void update_configuration();
};
