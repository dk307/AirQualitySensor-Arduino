#pragma once

#include "ui_screen_with_sensor_panel.h"

#include "sensor.h"

class ui_main_screen final : public ui_screen_with_sensor_panel
{
public:
    using ui_screen_with_sensor_panel::ui_screen_with_sensor_panel;

    void init() override
    {
        ui_screen_with_sensor_panel::init();

        lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);

        const int x_pad = 9;
        const int y_pad = 10;
        const int big_panel_w = (screen_width - x_pad * 3) / 2;
        const int big_panel_h = 150;
        const int small_panel_w = (screen_width - x_pad * 5) / 4;
        const int small_panel_h = 81;

        panel_and_labels[static_cast<size_t>(sensor_id_index::pm_2_5)] =
            create_big_panel(sensor_id_index::pm_2_5, x_pad, y_pad, big_panel_w, big_panel_h);
        panel_and_labels[static_cast<size_t>(sensor_id_index::voc)] =
            create_big_panel(sensor_id_index::voc, x_pad * 2 + big_panel_w, y_pad, big_panel_w, big_panel_h);

        panel_and_labels[static_cast<size_t>(sensor_id_index::pm_10)] =
            create_small_panel(sensor_id_index::pm_10, x_pad, big_panel_h + y_pad * 2, small_panel_w, small_panel_h);
        panel_and_labels[static_cast<size_t>(sensor_id_index::pm_4)] =
            create_small_panel(sensor_id_index::pm_4, x_pad * 2 + small_panel_w, big_panel_h + y_pad * 2, small_panel_w, small_panel_h);
        panel_and_labels[static_cast<size_t>(sensor_id_index::pm_1)] =
            create_small_panel(sensor_id_index::pm_1, x_pad * 3 + small_panel_w * 2, big_panel_h + y_pad * 2, small_panel_w, small_panel_h);
        panel_and_labels[static_cast<size_t>(sensor_id_index::eCO2)] =
            create_small_panel(sensor_id_index::eCO2, x_pad * 4 + small_panel_w * 3, big_panel_h + y_pad * 2, small_panel_w, small_panel_h);

        panel_and_labels[static_cast<size_t>(sensor_id_index::temperatureF)] =
            create_temperature_panel(sensor_id_index::temperatureF, 10, -10);
        panel_and_labels[static_cast<size_t>(sensor_id_index::humidity)] =
            create_humidity_panel(sensor_id_index::humidity, -10, -10);

        lv_obj_add_event_cb(screen, event_callback<ui_main_screen, &ui_main_screen::event_screen>, LV_EVENT_ALL, this);
        log_d("Main screen init done");
    }

    void set_sensor_value(sensor_id_index index, const std::optional<sensor_value::value_type> &value)
    {
        log_d("Updating sensor %d to %d in main screen", index, value.value_or(-1));
        const auto &pair = panel_and_labels.at(static_cast<size_t>(index));
        set_value_in_panel(pair, index, value);
    }

    void show_screen()
    {
        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }

private:
    std::array<panel_and_label, total_sensors> panel_and_labels;

    panel_and_label create_big_panel(sensor_id_index index,
                                     lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w, lv_coord_t h)
    {
        auto panel = create_panel(x_ofs, y_ofs, w, h, 20);

        auto label = lv_label_create(panel);
        lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

        lv_label_set_text_static(label, sensor_definitions[static_cast<uint8_t>(index)].get_name());
        lv_obj_set_style_text_color(label, off_black_color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

        auto value_label = lv_label_create(panel);
        lv_obj_set_size(value_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

        lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL);
        lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(value_label, fonts->font_montserrat_light_numbers_112, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(value_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);

        add_panel_callback_event(panel, index);

        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 9);
        lv_obj_align(value_label, LV_ALIGN_BOTTOM_MID, 0, -9);

        panel_and_label pair{panel, value_label};
        set_default_value_in_panel(pair);

        return pair;
    }

    lv_obj_t *create_panel(lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w, lv_coord_t h, lv_coord_t radius)
    {
        auto panel = lv_obj_create(screen);
        lv_obj_set_size(panel, w, h);
        lv_obj_align(panel, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
        lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(panel, false, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_radius(panel, radius, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_padding_zero(panel);

        lv_obj_add_flag(panel, LV_OBJ_FLAG_EVENT_BUBBLE);
        return panel;
    }

    panel_and_label create_small_panel(sensor_id_index index,
                                       lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w, lv_coord_t h)
    {
        auto panel = create_panel(x_ofs, y_ofs, w, h, 13);

        auto label = lv_label_create(panel);
        lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_label_set_text(label, sensor_definitions[static_cast<uint8_t>(index)].get_name());
        lv_obj_set_style_text_color(label, off_black_color, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(label, fonts->font_montserrat_medium_14, LV_PART_MAIN | LV_STATE_DEFAULT);

        auto value_label = lv_label_create(panel);
        lv_obj_set_size(value_label, lv_pct(100), LV_SIZE_CONTENT);

        lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL);
        lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(value_label, fonts->font_montserrat_regular_numbers_48, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(value_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);

        add_panel_callback_event(panel, index);

        panel_and_label pair{panel, value_label};
        set_default_value_in_panel(pair);

        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 5);
        lv_obj_align(value_label, LV_ALIGN_BOTTOM_MID, 0, -5);

        return pair;
    }

    panel_and_label create_temperature_panel(sensor_id_index index,
                                             lv_coord_t x_ofs, lv_coord_t y_ofs)
    {
        auto panel = lv_obj_create(screen);
        lv_obj_set_size(panel, 180, LV_SIZE_CONTENT);
        lv_obj_align(panel, LV_ALIGN_BOTTOM_LEFT, x_ofs, y_ofs);
        lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(panel, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(panel, LV_OBJ_FLAG_EVENT_BUBBLE);

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
        lv_obj_set_style_text_font(value_label, fonts->font_montserrat_medium_48, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(value_label, lv_color_hex(0x0), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(value_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text_fmt(value_label, "- %s", sensor_definitions[static_cast<uint8_t>(index)].get_unit());

        add_panel_callback_event(panel, index);
        return {nullptr, value_label};
    }

    panel_and_label create_humidity_panel(sensor_id_index index,
                                          lv_coord_t x_ofs, lv_coord_t y_ofs)
    {
        auto panel = lv_obj_create(screen);
        lv_obj_set_size(panel, 240, LV_SIZE_CONTENT);
        lv_obj_align(panel, LV_ALIGN_BOTTOM_RIGHT, x_ofs, y_ofs);
        lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(panel, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(panel, LV_OBJ_FLAG_EVENT_BUBBLE);

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
        lv_obj_set_style_text_font(value_label, fonts->font_montserrat_medium_48, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(value_label, lv_color_hex(0x0), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(value_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text_fmt(value_label, "- %s", sensor_definitions[static_cast<uint8_t>(index)].get_unit());

        add_panel_callback_event(panel, index);
        return {nullptr, value_label};
    }

    void event_screen(lv_event_t *e)
    {
        lv_event_code_t event_code = lv_event_get_code(e);
        lv_obj_t *target = lv_event_get_target(e);

        if (event_code == LV_EVENT_LONG_PRESSED)
        {
            inter_screen_interface.show_setting_screen();
        }
        else if (event_code == LV_EVENT_SCREEN_LOAD_START)
        {
            for (auto i = 0; i < total_sensors; i++)
            {
                set_sensor_value(static_cast<sensor_id_index>(i), ui_interface_instance.get_sensor_value(static_cast<sensor_id_index>(i)));
            }
        }
    }

    void add_panel_callback_event(lv_obj_t *panel, sensor_id_index index)
    {
        add_event_callback(
            panel, [this, index](lv_event_t *e)
            { 
                const auto code = lv_event_get_code(e);
                if (code == LV_EVENT_SHORT_CLICKED) {
                    inter_screen_interface.show_sensor_detail_screen(index);
                } },
            LV_EVENT_SHORT_CLICKED);
    }
};