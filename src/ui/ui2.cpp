#include "ui2.h"
#include "ui_interface.h"
#include "sensor.h"
#include <task_wrapper.h>

#include <tuple>
#include <memory>

ui EXT_RAM_ATTR ui::instance;

LV_IMG_DECLARE(ui_img_logo);

template <void (ui::*ftn)(lv_event_t *)>
void event_callback(lv_event_t *e)
{
    auto p_this = reinterpret_cast<ui *>(lv_event_get_user_data(e));
    (p_this->*ftn)(e);
}

void ui::event_mainscreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM)
    {
        lv_scr_load_anim(settings_screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
    }
}

void ui::bootscreen_screen_init(void)
{
    bootscreen = lv_obj_create(NULL);
    lv_obj_clear_flag(bootscreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(bootscreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(bootscreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bootscreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    bootlogo = lv_img_create(bootscreen);
    lv_img_set_src(bootlogo, &ui_img_logo);
    lv_obj_set_size(bootlogo, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(bootlogo, LV_ALIGN_CENTER, 0, -20);

    boot_message = lv_label_create(bootscreen);
    lv_obj_set_size(boot_message, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(boot_message, LV_ALIGN_CENTER, 0, 60);
    lv_label_set_text(boot_message, "Starting");
    lv_obj_set_style_text_color(boot_message, lv_color_hex(0xFCFEFC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(boot_message, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
}

ui::panel_and_label ui::main_screen_create_big_panel(sensor_id_index index,
                                                     lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w, lv_coord_t h)
{
    auto panel = lv_obj_create(main_screen);
    lv_obj_set_size(panel, w, h);
    lv_obj_align(panel, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    set_label_panel_color(panel, 0);

    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(panel, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(panel, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_padding_zero(panel);

    const uint8_t extra_y = 7;
    auto label = lv_label_create(panel);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, extra_y);
    lv_label_set_text(label, sensor_definitions[static_cast<uint8_t>(index)].get_name());
    lv_obj_set_style_text_color(label, lv_color_hex(0x1E1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, font_large, LV_PART_MAIN | LV_STATE_DEFAULT);

    auto value_label = lv_label_create(panel);
    lv_obj_set_size(value_label, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_align(value_label, LV_ALIGN_TOP_MID, 0, extra_y + 30);
    lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(value_label, font_montserrat_light_numbers_112, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(value_label, "-");

    return {panel, value_label};
}

ui::panel_and_label ui::main_screen_create_small_panel(sensor_id_index index,
                                                       lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w, lv_coord_t h)
{
    auto panel = lv_obj_create(main_screen);
    lv_obj_set_size(panel, w, h);
    lv_obj_align(panel, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    set_label_panel_color(panel, 0);

    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(panel, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(panel, 13, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_padding_zero(panel);

    const uint8_t extra_y = 6;
    auto label = lv_label_create(panel);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, extra_y);
    lv_label_set_text(label, sensor_definitions[static_cast<uint8_t>(index)].get_name());
    lv_obj_set_style_text_color(label, lv_color_hex(0x1E1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, font_normal, LV_PART_MAIN | LV_STATE_DEFAULT);

    auto value_label = lv_label_create(panel);
    lv_obj_set_size(value_label, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_align(value_label, LV_ALIGN_TOP_MID, 0, extra_y + 22);
    lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(value_label, font_montserrat_light_numbers_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(value_label, "-");

    return {panel, value_label};
}

ui::panel_and_label ui::main_screen_create_temperature_panel(sensor_id_index index,
                                                             lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    auto panel = lv_obj_create(main_screen);
    lv_obj_set_size(panel, 240, LV_SIZE_CONTENT);
    lv_obj_align(panel, LV_ALIGN_BOTTOM_LEFT, x_ofs, y_ofs);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_padding_zero(panel);

    auto image = lv_img_create(panel);
    lv_img_set_src(image, "S:display/image/temperature.png");
    lv_obj_align(image, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    auto value_label = lv_label_create(panel);
    lv_obj_set_size(value_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(value_label, LV_ALIGN_BOTTOM_LEFT, 48, 0);
    lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(value_label, font_montserrat_bold_numbers_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(value_label, lv_color_hex(0x0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(value_label, "- %s", sensor_definitions[static_cast<uint8_t>(index)].get_unit());

    return {nullptr, value_label};
}

ui::panel_and_label ui::main_screen_create_humidity_panel(sensor_id_index index,
                                                          lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    auto panel = lv_obj_create(main_screen);
    lv_obj_set_size(panel, 240, LV_SIZE_CONTENT);
    lv_obj_align(panel, LV_ALIGN_BOTTOM_RIGHT, x_ofs, y_ofs);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_padding_zero(panel);

    auto image = lv_img_create(panel);
    lv_img_set_src(image, "S:display/image/humidity.png");
    lv_obj_align(image, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    auto value_label = lv_label_create(panel);
    lv_obj_set_size(value_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(value_label, LV_ALIGN_BOTTOM_RIGHT, -48, 0);
    lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(value_label, font_montserrat_bold_numbers_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(value_label, lv_color_hex(0x0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(value_label, "- %s", sensor_definitions[static_cast<uint8_t>(index)].get_unit());

    return {nullptr, value_label};
}

void ui::main_screen_screen_init(void)
{
    main_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::pm_2_5)] = main_screen_create_big_panel(sensor_id_index::pm_2_5, 10, 10);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::voc)] = main_screen_create_big_panel(sensor_id_index::voc, 245, 10);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::pm_10)] = main_screen_create_small_panel(sensor_id_index::pm_10, 10, 160);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::pm_4)] = main_screen_create_small_panel(sensor_id_index::pm_4, 127, 160);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::pm_1)] = main_screen_create_small_panel(sensor_id_index::pm_1, 245, 160);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::eCO2)] = main_screen_create_small_panel(sensor_id_index::eCO2, 361, 160);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::temperatureF)] =
        main_screen_create_temperature_panel(sensor_id_index::temperatureF, 10, -12);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::humidity)] =
        main_screen_create_humidity_panel(sensor_id_index::humidity, -10, -12);

    lv_obj_add_event_cb(main_screen, event_callback<&ui::event_mainscreen>, LV_EVENT_ALL, this);
    log_d("Main screen init done");
}

void ui::load_information()
{
    log_d("updating info table");
    const auto data = ui_interface::instance.get_information_table();

    lv_table_set_col_cnt(settings_screen_tab_information_table, 2);
    lv_table_set_row_cnt(settings_screen_tab_information_table, data.size());

    lv_table_set_col_width(settings_screen_tab_information_table, 0, 140);
    lv_table_set_col_width(settings_screen_tab_information_table, 1, 430 - 140);

    for (auto i = 0; i < data.size(); i++)
    {
        lv_table_set_cell_value(settings_screen_tab_information_table, i, 0, std::get<0>(data[i]).c_str());
        lv_table_set_cell_value(settings_screen_tab_information_table, i, 1, std::get<1>(data[i]).c_str());
    }

    lv_slider_set_value(settings_screen_tab_settings_brightness_slider, ui_interface::instance.get_manual_screen_brightness(), LV_ANIM_OFF);
}

void ui::settings_screen_events_callback(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        log_d("setting screen shown");
        load_information();
        information_refresh_task = std::make_unique<task_wrapper>([this]
                                                                  {
                                                                      do
                                                                      {
                                                                          // log_d("Core:%d", xPortGetCoreID());
                                                                          load_information();
                                                                          vTaskDelay(1000);
                                                                      } while(true); });

        information_refresh_task->spawn_arduino_main_core("ui info table refresh");
    }
    else if (event_code == LV_EVENT_SCREEN_UNLOADED)
    {
        log_d("setting screen hidden");
        information_refresh_task.reset();
    }
    else if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM)
    {
        lv_scr_load_anim(main_screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
    }
}

void ui::settings_screen_tab_settings_brightness_slider_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        const auto value = lv_slider_get_value(settings_screen_tab_settings_brightness_slider);
        ui_interface::instance.set_manual_screen_brightness(value);
    }
}

void ui::settings_screen_screen_init(void)
{
    settings_screen = lv_obj_create(NULL);

    auto settings_screen_tab = lv_tabview_create(settings_screen, LV_DIR_TOP, 45);
    lv_obj_set_style_text_font(settings_screen, font_normal, 0);

    lv_obj_add_event_cb(settings_screen, event_callback<&ui::settings_screen_events_callback>, LV_EVENT_ALL, this);

    // Settings tab
    {
        auto settings_screen_tab_settings = lv_tabview_add_tab(settings_screen_tab, "Settings");

        // Settings - Brightness panel
        {
            static const lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
            static const lv_coord_t grid_main_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

            auto brightness_panel = lv_obj_create(settings_screen_tab_settings);
            lv_obj_set_size(brightness_panel, lv_pct(100), LV_SIZE_CONTENT);
            lv_obj_set_grid_dsc_array(brightness_panel, grid_main_col_dsc, grid_main_row_dsc);

            // label
            auto brightness_panel_label = lv_label_create(brightness_panel);

            lv_label_set_text(brightness_panel_label, "Screen Brightness");
            lv_obj_add_style(brightness_panel_label, &style_title, 0);
            lv_obj_set_grid_cell(brightness_panel_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 0, 1);

            // auto switch
            auto auto_brightness_switch = lv_switch_create(brightness_panel);

            auto auto_brightness_switch_label = lv_label_create(brightness_panel);
            lv_label_set_text(auto_brightness_switch_label, "Auto");
            lv_obj_add_style(auto_brightness_switch_label, &style_label_default, 0);
            lv_obj_set_grid_cell(auto_brightness_switch, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
            lv_obj_set_grid_cell(auto_brightness_switch_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

            // slider
            settings_screen_tab_settings_brightness_slider = lv_slider_create(brightness_panel);
            lv_obj_set_width(settings_screen_tab_settings_brightness_slider, lv_pct(97));
            lv_slider_set_range(settings_screen_tab_settings_brightness_slider, 1, 255);
            lv_obj_add_event_cb(settings_screen_tab_settings_brightness_slider,
                                event_callback<&ui::settings_screen_tab_settings_brightness_slider_event_cb>, LV_EVENT_VALUE_CHANGED, this);
            lv_obj_refresh_ext_draw_size(settings_screen_tab_settings_brightness_slider);
            lv_obj_set_grid_cell(settings_screen_tab_settings_brightness_slider, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 2, 1);
        }
    }

    // Information tab
    {
        auto settings_screen_tab_information = lv_tabview_add_tab(settings_screen_tab, "Information");

        settings_screen_tab_information_table = lv_table_create(settings_screen_tab_information);
        lv_obj_set_size(settings_screen_tab_information_table, lv_pct(100), LV_SIZE_CONTENT);
    }
}

void ui::load_from_sd_card()
{
    if (lv_fs_is_ready('S'))
    {
        log_i("lv fs is ready. Loading from SD Card");
    }
    else
    {
        log_e("lv fs not ready");
    }

    log_d("1");
    font_montserrat_light_numbers_48 = lv_font_load("S:display/font/montserrat_light_numbers_48.bin");
    log_d("2");
    font_montserrat_light_numbers_96 = lv_font_load("S:display/font/montserrat_light_numbers_96.bin");
    log_d("3");
    font_montserrat_light_numbers_112 = lv_font_load("S:display/font/montserrat_light_numbers_112.bin");
    log_d("4");
    font_montserrat_bold_numbers_48 = lv_font_load("S:display/font/montserrat_bold_numbers_48.bin");

    log_d("Loaded From SD Card");
}

void ui::init()
{
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_LIGHT_BLUE), lv_palette_main(LV_PALETTE_RED),
                                              false, LV_FONT_DEFAULT);

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    lv_style_init(&style_label_default);
    lv_style_set_text_font(&style_label_default, font_normal);

    lv_disp_set_theme(dispp, theme);
    bootscreen_screen_init();

    lv_disp_load_scr(bootscreen);

    inline_loop(100);

    log_i("Loaded boot screen");

    lv_label_set_text(boot_message, "Loading from SD Card");
    inline_loop(50);

    load_from_sd_card(); // might take some time

    main_screen_screen_init();
    settings_screen_screen_init();
}

void ui::inline_loop(uint64_t maxWait)
{
    const auto now = millis();
    while (millis() - now < maxWait)
    {
        lv_timer_handler();
        delay(5);
    }
}

void ui::set_label_panel_color(lv_obj_t *panel, uint64_t level)
{
    uint32_t color;
    uint32_t color_grad;

    switch (level)
    {
    case 0:
        color = 0x4BD175; // green
        color_grad = 0x228D44;
        break;
    case 1:
        color = 0x767C30; // yellow
        color_grad = 0xBFCC20;
        break;
    case 2:
        color = 0xEC9706; // orange
        color_grad = 0xED7117;
        break;
    case 3:
        color = 0xE3242B; // red
        color_grad = 0x900D09;
        break;
    case 4:
        color = 0x710193; // purple
        color_grad = 0xA32CC4;
        break;
    default:
    case 5:
        color = 0x940606; // Maroon
        color_grad = 0xc30808;
        break;
    }

    lv_obj_set_style_bg_color(panel, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(panel, lv_color_hex(color_grad), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void ui::set_sensor_value(sensor_id_index id, uint16_t value, sensor_level level)
{
    log_v("Updating sensor %d to %d", id, value);
    const auto &pair = main_screen_panel_and_label.at(static_cast<size_t>(id));
    if (pair.panel)
    {
        set_label_panel_color(pair.panel, level);
    }

    if (pair.label)
    {
        if (!pair.panel)
        {
            lv_label_set_text_fmt(pair.label, "%d%s", value, sensor_definitions[static_cast<uint8_t>(id)].get_unit());
        }
        else
        {
            lv_label_set_text_fmt(pair.label, "%d", value);
        }
    }
}

void ui::update_boot_message(const std::string &message)
{
    lv_label_set_text(boot_message, message.c_str());
    inline_loop(50);
}

void ui::set_main_screen()
{
    lv_disp_load_scr(main_screen);
}

void ui::set_padding_zero(lv_obj_t *obj)
{
    lv_obj_set_style_pad_left(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}