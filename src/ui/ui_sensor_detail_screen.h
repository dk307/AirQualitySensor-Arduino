#pragma once

#include "ui_screen_with_sensor_panel.h"
#include "sensor.h"
#include "ntp_time.h"

class ui_sensor_detail_screen final : public ui_screen_with_sensor_panel
{
public:
    using ui_screen_with_sensor_panel::ui_screen_with_sensor_panel;

    void init() override
    {
        ui_screen::init();

        const auto x_pad = 6;
        const auto y_pad = 4;

        lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);

        sensor_detail_screen_top_label =
            create_sensor_detail_screen_label(screen, fonts->font_montserrat_medium_48, LV_ALIGN_TOP_MID, 0, y_pad, lv_color_black());

        sensor_detail_screen_top_label_units =
            create_sensor_detail_screen_label(screen, fonts->font_montserrat_medium_units_18, LV_ALIGN_TOP_RIGHT, -2 * x_pad, y_pad + 10, off_black_color);

        lv_obj_set_style_text_align(sensor_detail_screen_top_label_units, LV_TEXT_ALIGN_AUTO, LV_PART_MAIN | LV_STATE_DEFAULT);

        const auto top_y_margin = lv_obj_get_style_text_font(sensor_detail_screen_top_label, LV_PART_MAIN | LV_STATE_DEFAULT)->line_height + y_pad + 2;

        const auto panel_w = (screen_width - x_pad * 2) / 5;
        const auto panel_h = (screen_height - y_pad * 4 - top_y_margin) / 4;

        // first label is up by y_pad
        sensor_detail_screen_label_and_unit_labels[label_and_unit_label_current_index] =
            create_detail_screen_panel("Current",
                                       LV_ALIGN_TOP_RIGHT, -x_pad, top_y_margin,
                                       panel_w, panel_h);

        sensor_detail_screen_label_and_unit_labels[label_and_unit_label_average_index] =
            create_detail_screen_panel("Average",
                                       LV_ALIGN_TOP_RIGHT, -x_pad, top_y_margin + y_pad + panel_h,
                                       panel_w, panel_h);

        sensor_detail_screen_label_and_unit_labels[label_and_unit_label_min_index] =
            create_detail_screen_panel("Minimum",
                                       LV_ALIGN_TOP_RIGHT, -x_pad, top_y_margin + (y_pad + panel_h) * 2,
                                       panel_w, panel_h);

        sensor_detail_screen_label_and_unit_labels[label_and_unit_label_max_index] =
            create_detail_screen_panel("Maximum",
                                       LV_ALIGN_TOP_RIGHT, -x_pad, top_y_margin + (y_pad + panel_h) * 3,
                                       panel_w, panel_h);

        {
            const auto extra_chart_x = 45;
            sensor_detail_screen_chart = lv_chart_create(screen);
            lv_obj_set_size(sensor_detail_screen_chart,
                            screen_width - panel_w - x_pad * 3 - extra_chart_x - 10,
                            screen_height - top_y_margin - 3 * y_pad - 20);
            lv_obj_align(sensor_detail_screen_chart, LV_ALIGN_TOP_LEFT, x_pad + extra_chart_x, y_pad + top_y_margin);
            lv_obj_set_style_size(sensor_detail_screen_chart, 0, LV_PART_INDICATOR);
            sensor_detail_screen_chart_series =
                lv_chart_add_series(sensor_detail_screen_chart, lv_palette_lighten(LV_PALETTE_GREEN, 2), LV_CHART_AXIS_PRIMARY_Y);
            lv_obj_set_style_text_font(sensor_detail_screen_chart, fonts->font_montserrat_medium_14, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_chart_set_axis_tick(sensor_detail_screen_chart, LV_CHART_AXIS_PRIMARY_Y, 5, 1, 3, 1, true, 200);
            lv_chart_set_axis_tick(sensor_detail_screen_chart, LV_CHART_AXIS_PRIMARY_X, 10, 5, chart_total_x_ticks, 1, true, 50);

            lv_chart_set_div_line_count(sensor_detail_screen_chart, 3, 3);
            lv_obj_set_style_border_width(sensor_detail_screen_chart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_event_cb(sensor_detail_screen_chart,
                                event_callback<ui_sensor_detail_screen, &ui_sensor_detail_screen::chart_draw_event_cb>,
                                LV_EVENT_DRAW_PART_BEGIN, this);
        }

        create_close_button_to_main_screen(screen, LV_ALIGN_BOTTOM_LEFT, 15, -15);

        log_d("Sensor detail init done");
    }

    void set_sensor_value(sensor_id_index index, const std::optional<sensor_value::value_type> &value)
    {
        if (lv_scr_act() == screen)
        {
            if (lv_obj_get_user_data(screen) == reinterpret_cast<void *>(index))
            {
                log_d("Updating sensor %d to %d in details screen", index, value.value_or(-1));
                detail_screen_current_values(index, value);
            }
        }
    }

    void show_screen(sensor_id_index index)
    {
        log_i("Panel pressed for sensor index:%d", index);
        lv_obj_set_user_data(screen, reinterpret_cast<void *>(index));

        lv_label_set_text(sensor_detail_screen_top_label, sensor_definitions[static_cast<uint8_t>(index)].get_name());
        lv_label_set_text_static(sensor_detail_screen_top_label_units, sensor_definitions[static_cast<uint8_t>(index)].get_unit());

        const auto value = ui_interface_instance.get_sensor_value(index);
        detail_screen_current_values(index, value);

        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
    }

private:
    lv_obj_t *sensor_detail_screen_top_label;
    lv_obj_t *sensor_detail_screen_top_label_units;
    lv_obj_t *sensor_detail_screen_chart;
    lv_chart_series_t *sensor_detail_screen_chart_series;
    std::vector<sensor_value::value_type, psram::allocator<sensor_value::value_type>> sensor_detail_screen_chart_series_data;
    std::optional<time_t> sensor_detail_screen_chart_series_data_time;
    const static uint8_t chart_total_x_ticks = 4;

    std::array<panel_and_label, 4> sensor_detail_screen_label_and_unit_labels;
    const size_t label_and_unit_label_current_index = 0;
    const size_t label_and_unit_label_average_index = 1;
    const size_t label_and_unit_label_min_index = 2;
    const size_t label_and_unit_label_max_index = 3;

    panel_and_label create_detail_screen_panel(const char *label_text,
                                               lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs,
                                               lv_coord_t w, lv_coord_t h)
    {
        auto panel = lv_obj_create(screen);
        lv_obj_set_size(panel, w, h);
        lv_obj_align(panel, align, x_ofs, y_ofs);
        lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_clip_corner(panel, false, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_set_style_radius(panel, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
        set_padding_zero(panel);

        auto current_static_label =
            create_sensor_detail_screen_label(panel, fonts->font_montserrat_medium_14, LV_ALIGN_TOP_MID, 0, 3, lv_color_black());

        lv_label_set_text_static(current_static_label, label_text);

        auto value_label =
            create_sensor_detail_screen_label(panel, fonts->font_montserrat_regular_numbers_40, LV_ALIGN_BOTTOM_MID,
                                              0, -3, lv_color_white());

        return {panel, value_label};
    }

    static lv_obj_t *create_sensor_detail_screen_label(lv_obj_t *parent, const lv_font_t *font,
                                                       lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_color_t color)
    {
        auto *label = lv_label_create(parent);
        lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_align(label, align, x_ofs, y_ofs);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(label, font, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label, color, LV_PART_MAIN | LV_STATE_DEFAULT);

        return label;
    }

    void chart_draw_event_cb(lv_event_t *e)
    {
        lv_obj_t *obj = lv_event_get_target(e);

        /*Add the faded area before the lines are drawn*/
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
        if (dsc->part == LV_PART_ITEMS)
        {
            if (!dsc->p1 || !dsc->p2)
                return;

            /*Add a line mask that keeps the area below the line*/
            lv_draw_mask_line_param_t line_mask_param;
            lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y,
                                          LV_DRAW_MASK_LINE_SIDE_BOTTOM);
            int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

            /*Add a fade effect: transparent bottom covering top*/
            lv_coord_t h = lv_obj_get_height(obj);
            lv_draw_mask_fade_param_t fade_mask_param;
            lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER, obj->coords.y1 + h / 8, LV_OPA_TRANSP,
                                   obj->coords.y2);
            int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

            /*Draw a rectangle that will be affected by the mask*/
            lv_draw_rect_dsc_t draw_rect_dsc;
            lv_draw_rect_dsc_init(&draw_rect_dsc);
            draw_rect_dsc.bg_opa = LV_OPA_80;
            draw_rect_dsc.bg_color = dsc->line_dsc->color;

            lv_area_t a;
            a.x1 = dsc->p1->x;
            a.x2 = dsc->p2->x - 1;
            a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
            a.y2 = obj->coords.y2;
            lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

            /*Remove the masks*/
            lv_draw_mask_free_param(&line_mask_param);
            lv_draw_mask_free_param(&fade_mask_param);
            lv_draw_mask_remove_id(line_mask_id);
            lv_draw_mask_remove_id(fade_mask_id);
        }
        /*Hook the division lines too*/
        else if (dsc->part == LV_PART_MAIN)
        {
            if (dsc->line_dsc == NULL || dsc->p1 == NULL || dsc->p2 == NULL)
                return;

            /*Vertical line*/
            if (dsc->p1->x == dsc->p2->x)
            {
                dsc->line_dsc->color = lv_palette_lighten(LV_PALETTE_GREY, 1);
                if (dsc->id == 3)
                {
                    dsc->line_dsc->width = 2;
                    dsc->line_dsc->dash_gap = 0;
                    dsc->line_dsc->dash_width = 0;
                }
                else
                {
                    dsc->line_dsc->width = 1;
                    dsc->line_dsc->dash_gap = 6;
                    dsc->line_dsc->dash_width = 6;
                }
            }
            /*Horizontal line*/
            else
            {
                if (dsc->id == 2)
                {
                    dsc->line_dsc->width = 2;
                    dsc->line_dsc->dash_gap = 0;
                    dsc->line_dsc->dash_width = 0;
                }
                else
                {
                    dsc->line_dsc->width = 2;
                    dsc->line_dsc->dash_gap = 6;
                    dsc->line_dsc->dash_width = 6;
                }

                if (dsc->id == 1 || dsc->id == 3)
                {
                    dsc->line_dsc->color = lv_palette_main(LV_PALETTE_GREEN);
                }
                else
                {
                    dsc->line_dsc->color = lv_palette_lighten(LV_PALETTE_GREY, 1);
                }
            }
        }
        else if (dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X)
        {
            if (sensor_detail_screen_chart_series_data_time.has_value() && sensor_detail_screen_chart_series_data.size())
            {
                const auto data_interval_seconds = (sensor_detail_screen_chart_series_data.size() * 60) / sensor_history::reads_per_minute;
                const float interval = float(chart_total_x_ticks - 1 - dsc->value) / (chart_total_x_ticks - 1);
                // log_i("total seconds :%d,  Series length: %d,  %f", data_interval_seconds, sensor_detail_screen_chart_series_data.size(), interval);
                const time_t tick_time = sensor_detail_screen_chart_series_data_time.value() - (data_interval_seconds * interval);

                tm t{};
                localtime_r(&tick_time, &t);
                strftime(dsc->text, 32, "%I:%M%p", &t);
            }
            else
            {
                strcpy(dsc->text, "-");
            }
        }
    }

    void detail_screen_current_values(sensor_id_index index, const std::optional<sensor_value::value_type> &value)
    {
        set_value_in_panel(sensor_detail_screen_label_and_unit_labels[label_and_unit_label_current_index], index, value);

        sensor_detail_screen_chart_series_data_time = ntp_time::instance.get_local_time();

        time_t t = sensor_detail_screen_chart_series_data_time.value_or((time_t)0);

        auto &&sensor_info = ui_interface_instance.get_sensor_detail_info(index);
        if (sensor_info.last_x_min_stats.has_value())
        {
            auto &&stats = sensor_info.last_x_min_stats.value();

            lv_chart_set_type(sensor_detail_screen_chart, LV_CHART_TYPE_LINE);

            set_value_in_panel(sensor_detail_screen_label_and_unit_labels[label_and_unit_label_average_index], index, stats.mean);
            set_value_in_panel(sensor_detail_screen_label_and_unit_labels[label_and_unit_label_min_index], index, stats.min);
            set_value_in_panel(sensor_detail_screen_label_and_unit_labels[label_and_unit_label_max_index], index, stats.max);

            auto &&values = sensor_info.last_x_min_values;
            lv_chart_set_point_count(sensor_detail_screen_chart, values.size());
            lv_chart_set_range(sensor_detail_screen_chart, LV_CHART_AXIS_PRIMARY_Y, stats.min, stats.max);

            sensor_detail_screen_chart_series_data = std::move(values);

            lv_chart_set_ext_y_array(sensor_detail_screen_chart, sensor_detail_screen_chart_series,
                                     sensor_detail_screen_chart_series_data.data());
            (sensor_detail_screen_chart, 256);
        }
        else
        {
            log_d("No stats for %d", index);
            set_default_value_in_panel(sensor_detail_screen_label_and_unit_labels[label_and_unit_label_average_index]);
            set_default_value_in_panel(sensor_detail_screen_label_and_unit_labels[label_and_unit_label_min_index]);
            set_default_value_in_panel(sensor_detail_screen_label_and_unit_labels[label_and_unit_label_max_index]);

            sensor_detail_screen_chart_series_data.clear();
            lv_chart_set_type(sensor_detail_screen_chart, LV_CHART_TYPE_NONE);
        }
    }
};