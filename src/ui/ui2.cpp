#include "ui2.h"
#include "ui_interface.h"
#include "config_manager.h"
#include "sensor.h"
#include "ntp_time.h"
#include <task_wrapper.h>

#include <tuple>
#include <memory>

LV_IMG_DECLARE(ui_img_logo);

const int screen_width = 480;
const int screen_height = 320;
const auto off_black_color = lv_color_hex(0x1E1E1E);
const auto black_color = lv_color_hex(0);
const auto white_color = lv_color_hex(0xFFFFFF);
const auto no_value_label = std::numeric_limits<uint8_t>::max();
const auto chart_total_x_ticks = 4;

template <void (ui::*ftn)(lv_event_t *)>
void event_callback(lv_event_t *e)
{
    auto p_this = reinterpret_cast<ui *>(lv_event_get_user_data(e));
    (p_this->*ftn)(e);
}

void event_callback_ftn(lv_event_t *e)
{
    auto p_ftn = reinterpret_cast<std::function<void(lv_event_t * e)> *>(lv_event_get_user_data(e));
    (*p_ftn)(e);
}

void ui::event_main_screen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE)
    {
        const auto gesture_dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if ((gesture_dir == LV_DIR_BOTTOM) || (gesture_dir == LV_DIR_TOP))
        {
            lv_scr_load_anim(settings_screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
        }
    }
    else if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        for (auto i = 0; i < total_sensors; i++)
        {
            set_sensor_value(static_cast<sensor_id_index>(i), ui_interface_instance.get_sensor_value(static_cast<sensor_id_index>(i)));
        }
    }
}

void ui::boot_screen_screen_init(void)
{
    boot_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(boot_screen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(boot_screen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(boot_screen, black_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(boot_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    boot_logo = lv_img_create(boot_screen);
    lv_img_set_src(boot_logo, &ui_img_logo);
    lv_obj_set_size(boot_logo, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(boot_logo, LV_ALIGN_CENTER, 0, -20);

    boot_message = lv_label_create(boot_screen);
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

    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(panel, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(panel, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_padding_zero(panel);

    auto label = lv_label_create(panel);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_label_set_text_static(label, sensor_definitions[static_cast<uint8_t>(index)].get_name());
    lv_obj_set_style_text_color(label, off_black_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, font_large, LV_PART_MAIN | LV_STATE_DEFAULT);

    auto value_label = lv_label_create(panel);
    lv_obj_set_size(value_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

    lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(value_label, font_montserrat_light_numbers_112, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(value_label, white_color, LV_PART_MAIN | LV_STATE_DEFAULT);

    const auto param = new std::pair<ui *, sensor_id_index>(this, index);
    add_panel_callback_event(panel, index);

    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 9);
    lv_obj_align(value_label, LV_ALIGN_BOTTOM_MID, 0, -9);

    panel_and_label pair{panel, value_label};
    set_default_value_in_panel(pair);

    return pair;
}

ui::panel_and_label ui::main_screen_create_small_panel(sensor_id_index index,
                                                       lv_coord_t x_ofs, lv_coord_t y_ofs, lv_coord_t w, lv_coord_t h)
{
    auto panel = lv_obj_create(main_screen);
    lv_obj_set_size(panel, w, h);
    lv_obj_align(panel, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_opa(panel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(panel, false, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(panel, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(panel, 13, LV_PART_MAIN | LV_STATE_DEFAULT);
    set_padding_zero(panel);

    auto label = lv_label_create(panel);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text(label, sensor_definitions[static_cast<uint8_t>(index)].get_name());
    lv_obj_set_style_text_color(label, off_black_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label, font_montserrat_medium_14, LV_PART_MAIN | LV_STATE_DEFAULT);

    auto value_label = lv_label_create(panel);
    lv_obj_set_size(value_label, lv_pct(100), LV_SIZE_CONTENT);

    lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(value_label, font_montserrat_regular_numbers_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(value_label, white_color, LV_PART_MAIN | LV_STATE_DEFAULT);

    add_panel_callback_event(panel, index);

    panel_and_label pair{panel, value_label};
    set_default_value_in_panel(pair);

    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_align(value_label, LV_ALIGN_BOTTOM_MID, 0, -5);

    return pair;
}

ui::panel_and_label ui::main_screen_create_temperature_panel(sensor_id_index index,
                                                             lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    auto panel = lv_obj_create(main_screen);
    lv_obj_set_size(panel, 180, LV_SIZE_CONTENT);
    lv_obj_align(panel, LV_ALIGN_BOTTOM_LEFT, x_ofs, y_ofs);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(panel, white_color, LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_obj_set_style_text_font(value_label, font_montserrat_medium_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(value_label, lv_color_hex(0x0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(value_label, white_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(value_label, "- %s", sensor_definitions[static_cast<uint8_t>(index)].get_unit());

    add_panel_callback_event(panel, index);
    return {nullptr, value_label};
}

ui::panel_and_label ui::main_screen_create_humidity_panel(sensor_id_index index,
                                                          lv_coord_t x_ofs, lv_coord_t y_ofs)
{
    auto panel = lv_obj_create(main_screen);
    lv_obj_set_size(panel, 240, LV_SIZE_CONTENT);
    lv_obj_align(panel, LV_ALIGN_BOTTOM_RIGHT, x_ofs, y_ofs);
    lv_obj_set_style_border_width(panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(panel, white_color, LV_PART_MAIN | LV_STATE_DEFAULT);
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
    lv_obj_set_style_text_font(value_label, font_montserrat_medium_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(value_label, lv_color_hex(0x0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(value_label, white_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(value_label, "- %s", sensor_definitions[static_cast<uint8_t>(index)].get_unit());

    add_panel_callback_event(panel, index);
    return {nullptr, value_label};
}

void ui::create_close_button_to_main_screen(lv_obj_t *parent)
{
    lv_obj_t *close_button = lv_btn_create(parent);
    lv_obj_add_flag(close_button, LV_OBJ_FLAG_FLOATING | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(close_button, lv_color_white(), LV_STATE_CHECKED);
    lv_obj_set_style_pad_all(close_button, 15, 0);
    lv_obj_set_style_radius(close_button, LV_RADIUS_CIRCLE, 0);

    lv_obj_set_style_shadow_width(close_button, 0, 0);
    lv_obj_set_style_bg_img_src(close_button, LV_SYMBOL_HOME, 0);

    lv_obj_set_size(close_button, LV_DPX(60), LV_DPX(60));
    lv_obj_align(close_button, LV_ALIGN_BOTTOM_LEFT, LV_DPX(15), -LV_DPX(15));

    add_event_callback(
        close_button, [this](lv_event_t *e)
        { if (e->code == LV_EVENT_CLICKED) {
             lv_scr_load_anim(main_screen, LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, false);
         } },
        LV_EVENT_PRESSED);
}

void ui::main_screen_screen_init(void)
{
    main_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(main_screen, white_color, LV_PART_MAIN | LV_STATE_DEFAULT);

    const int x_pad = 9;
    const int y_pad = 10;
    const int big_panel_w = (screen_width - x_pad * 3) / 2;
    const int big_panel_h = 150;
    const int small_panel_w = (screen_width - x_pad * 5) / 4;
    const int small_panel_h = 81;

    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::pm_2_5)] =
        main_screen_create_big_panel(sensor_id_index::pm_2_5, x_pad, y_pad, big_panel_w, big_panel_h);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::voc)] =
        main_screen_create_big_panel(sensor_id_index::voc, x_pad * 2 + big_panel_w, y_pad, big_panel_w, big_panel_h);

    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::pm_10)] =
        main_screen_create_small_panel(sensor_id_index::pm_10, x_pad, big_panel_h + y_pad * 2, small_panel_w, small_panel_h);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::pm_4)] =
        main_screen_create_small_panel(sensor_id_index::pm_4, x_pad * 2 + small_panel_w, big_panel_h + y_pad * 2, small_panel_w, small_panel_h);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::pm_1)] =
        main_screen_create_small_panel(sensor_id_index::pm_1, x_pad * 3 + small_panel_w * 2, big_panel_h + y_pad * 2, small_panel_w, small_panel_h);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::eCO2)] =
        main_screen_create_small_panel(sensor_id_index::eCO2, x_pad * 4 + small_panel_w * 3, big_panel_h + y_pad * 2, small_panel_w, small_panel_h);

    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::temperatureF)] =
        main_screen_create_temperature_panel(sensor_id_index::temperatureF, 10, -10);
    main_screen_panel_and_label[static_cast<size_t>(sensor_id_index::humidity)] =
        main_screen_create_humidity_panel(sensor_id_index::humidity, -10, -10);

    lv_obj_add_event_cb(main_screen, event_callback<&ui::event_main_screen>, LV_EVENT_ALL, this);
    log_d("Main screen init done");
}

lv_obj_t *ui::create_sensor_detail_screen_label(lv_obj_t *parent, const lv_font_t *font, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, lv_color_t color)
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

lv_coord_t ui::get_label_height(lv_obj_t *label)
{
    return lv_obj_get_style_text_font(label, LV_PART_MAIN | LV_STATE_DEFAULT)->line_height;
}

ui::panel_and_label ui::create_detail_screen_panel(const char *label_text,
                                                   lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs,
                                                   lv_coord_t w, lv_coord_t h)
{
    auto panel = lv_obj_create(sensor_detail_screen);
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
        create_sensor_detail_screen_label(panel, font_montserrat_medium_14, LV_ALIGN_TOP_MID, 0, 3, black_color);

    lv_label_set_text_static(current_static_label, label_text);

    auto value_label =
        create_sensor_detail_screen_label(panel, font_montserrat_regular_numbers_40, LV_ALIGN_BOTTOM_MID,
                                          0, -3, white_color);

    return {panel, value_label};
}

void ui::chart_draw_event_cb(lv_event_t *e)
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

void ui::sensor_detail_screen_init(void)
{
    const auto x_pad = 6;
    const auto y_pad = 4;

    sensor_detail_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(sensor_detail_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(sensor_detail_screen, white_color, LV_PART_MAIN | LV_STATE_DEFAULT);

    sensor_detail_screen_top_label =
        create_sensor_detail_screen_label(sensor_detail_screen, font_montserrat_medium_48, LV_ALIGN_TOP_MID, 0, y_pad, black_color);

    sensor_detail_screen_top_label_units =
        create_sensor_detail_screen_label(sensor_detail_screen, font_montserrat_medium_units_18, LV_ALIGN_TOP_RIGHT, -2 * x_pad, y_pad + 10, off_black_color);

    lv_obj_set_style_text_align(sensor_detail_screen_top_label_units, LV_TEXT_ALIGN_AUTO, LV_PART_MAIN | LV_STATE_DEFAULT);

    const auto top_y_margin = get_label_height(sensor_detail_screen_top_label) + y_pad + 2;

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
        sensor_detail_screen_chart = lv_chart_create(sensor_detail_screen);
        lv_obj_set_size(sensor_detail_screen_chart,
                        screen_width - panel_w - x_pad * 3 - extra_chart_x - 10,
                        screen_height - top_y_margin - 3 * y_pad - 20);
        lv_obj_align(sensor_detail_screen_chart, LV_ALIGN_TOP_LEFT, x_pad + extra_chart_x, y_pad + top_y_margin);
        lv_obj_set_style_size(sensor_detail_screen_chart, 0, LV_PART_INDICATOR);
        sensor_detail_screen_chart_series =
            lv_chart_add_series(sensor_detail_screen_chart, lv_palette_lighten(LV_PALETTE_GREEN, 2), LV_CHART_AXIS_PRIMARY_Y);
        lv_obj_set_style_text_font(sensor_detail_screen_chart, font_montserrat_medium_14, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_chart_set_axis_tick(sensor_detail_screen_chart, LV_CHART_AXIS_PRIMARY_Y, 5, 1, 3, 1, true, 200);
        lv_chart_set_axis_tick(sensor_detail_screen_chart, LV_CHART_AXIS_PRIMARY_X, 10, 5, chart_total_x_ticks, 1, true, 50);

        lv_chart_set_div_line_count(sensor_detail_screen_chart, 3, 3);
        lv_obj_set_style_border_width(sensor_detail_screen_chart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_event_cb(sensor_detail_screen_chart, event_callback<&ui::chart_draw_event_cb>, LV_EVENT_DRAW_PART_BEGIN, this);
    }

    create_close_button_to_main_screen(sensor_detail_screen);

    log_d("Sensor detail init done");
}

void ui::load_information()
{
    log_v("updating info table");
    const auto data = ui_interface_instance.get_information_table();

    lv_table_set_col_cnt(settings_screen_tab_information_table, 2);
    lv_table_set_row_cnt(settings_screen_tab_information_table, data.size());

    lv_table_set_col_width(settings_screen_tab_information_table, 0, 140);
    lv_table_set_col_width(settings_screen_tab_information_table, 1, 430 - 140);

    for (auto i = 0; i < data.size(); i++)
    {
        lv_table_set_cell_value(settings_screen_tab_information_table, i, 0, std::get<0>(data[i]).c_str());
        lv_table_set_cell_value(settings_screen_tab_information_table, i, 1, std::get<1>(data[i]).c_str());
    }
}

void ui::settings_screen_events_callback(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        log_d("setting screen shown");
        load_information();
        update_configuration();
        information_refresh_task = std::make_unique<task_wrapper>([this]
                                                                  {
                                                                      do
                                                                      {
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
}

bool ui::settings_screen_screen_key_board_event_cb(lv_event_t *e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);

    if ((code == LV_EVENT_FOCUSED) || (code == LV_EVENT_CLICKED))
    {
        lv_obj_move_foreground(settings_screen_tab_settings_kb);
        lv_keyboard_set_textarea(settings_screen_tab_settings_kb, ta);
        lv_obj_clear_flag(settings_screen_tab_settings_kb, LV_OBJ_FLAG_HIDDEN);
    }
    else if ((code == LV_EVENT_DEFOCUSED) || (code == LV_EVENT_READY))
    {
        lv_keyboard_set_textarea(settings_screen_tab_settings_kb, NULL);
        lv_obj_add_flag(settings_screen_tab_settings_kb, LV_OBJ_FLAG_HIDDEN);

        return true;
    }
    return false;
}

void ui::settings_screen_screen_init(void)
{
    const auto lv_title_font = &lv_font_montserrat_16;
    settings_screen = lv_obj_create(NULL);

    auto settings_screen_tab = lv_tabview_create(settings_screen, LV_DIR_TOP, 45);
    lv_obj_set_style_text_font(settings_screen, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(settings_screen, event_callback<&ui::settings_screen_events_callback>, LV_EVENT_ALL, this);

    // Settings tab
    {
        auto settings_screen_tab_settings = lv_tabview_add_tab(settings_screen_tab, LV_SYMBOL_SETTINGS " Settings");

        settings_screen_tab_settings_kb = lv_keyboard_create(settings_screen);
        lv_obj_set_size(settings_screen_tab_settings_kb, screen_width, screen_height / 2);
        lv_obj_set_style_text_font(settings_screen_tab_settings_kb, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(settings_screen_tab_settings_kb, LV_OBJ_FLAG_HIDDEN);

        // Settings - other panel
        {
            const int y_pad = 15;

            auto other_settings_panel = lv_obj_create(settings_screen_tab_settings);
            lv_obj_set_size(other_settings_panel, lv_pct(100), LV_SIZE_CONTENT);

            lv_obj_t *last_obj = nullptr;
            {
                // hostname label
                auto host_name_text_area_label = lv_label_create(other_settings_panel);
                lv_label_set_text(host_name_text_area_label, "Hostname:");

                // hostname text area
                host_name_text_area = lv_textarea_create(other_settings_panel);
                lv_textarea_set_one_line(host_name_text_area, true);
                lv_obj_set_width(host_name_text_area, lv_pct(100));

                lv_obj_set_style_text_font(host_name_text_area_label, lv_title_font, LV_PART_MAIN | LV_STATE_DEFAULT);

                add_event_callback(host_name_text_area, [this](lv_event_t *e)
                                   {
                    if (settings_screen_screen_key_board_event_cb(e)) {
                        lv_obj_t *ta = lv_event_get_target(e);
                        const auto value = lv_textarea_get_text(ta);
                        if (!config::instance.data.get_host_name().equals(value)) {
                            config::instance.data.set_host_name(value);
                            config::instance.save();
                        }
                    } });
                lv_obj_align_to(host_name_text_area, host_name_text_area_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
                last_obj = host_name_text_area;
            }

            {
                // ntp server label
                auto ntp_server_text_area_label = lv_label_create(other_settings_panel);
                lv_label_set_text(ntp_server_text_area_label, "NTP Server:");
                lv_obj_set_style_text_font(ntp_server_text_area_label, lv_title_font, LV_PART_MAIN | LV_STATE_DEFAULT);

                // ntp server text area
                ntp_server_text_area = lv_textarea_create(other_settings_panel);
                lv_textarea_set_one_line(ntp_server_text_area, true);
                lv_obj_set_width(ntp_server_text_area, lv_pct(100));

                add_event_callback(ntp_server_text_area, [this](lv_event_t *e)
                                   {
                    if (settings_screen_screen_key_board_event_cb(e)) {
                        lv_obj_t *ta = lv_event_get_target(e);
                        const auto value = lv_textarea_get_text(ta);
                        if (!config::instance.data.get_host_name().equals(value)) {
                        config::instance.data.set_ntp_server(value);
                        config::instance.save();
                        }
                    } });

                lv_obj_align_to(ntp_server_text_area_label, last_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, y_pad);
                lv_obj_align_to(ntp_server_text_area, ntp_server_text_area_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
                last_obj = ntp_server_text_area;
            }

            {
                // ntp server refresh interval label
                auto ntp_server_refresh_interval_label = lv_label_create(other_settings_panel);
                lv_label_set_text(ntp_server_refresh_interval_label, "NTP Server sync interval (seconds):");
                lv_obj_set_style_text_font(ntp_server_refresh_interval_label, lv_title_font, LV_PART_MAIN | LV_STATE_DEFAULT);

                // ntp server refresh interval spin box
                ntp_server_refresh_interval_label_spinbox = lv_spinbox_create(other_settings_panel);
                lv_spinbox_set_range(ntp_server_refresh_interval_label_spinbox, 0, 3600);
                lv_spinbox_set_digit_format(ntp_server_refresh_interval_label_spinbox, 4, 0);

                lv_spinbox_step_prev(ntp_server_refresh_interval_label_spinbox);
                lv_obj_set_width(ntp_server_refresh_interval_label_spinbox, 150);

                add_event_callback(ntp_server_refresh_interval_label_spinbox, [this](lv_event_t *e)
                                   {
                                       const lv_event_code_t code = lv_event_get_code(e);
                                       if (code == LV_EVENT_VALUE_CHANGED)
                                       {
                                           const auto value = lv_spinbox_get_value(ntp_server_refresh_interval_label_spinbox) * 1000;
                                           if (config::instance.data.get_ntp_server_refresh_interval() != value) {
                                           config::instance.data.set_ntp_server_refresh_interval(value);
                                           config::instance.save();
                                           }
                                       } });

                const auto spin_box_height = lv_obj_get_height(ntp_server_refresh_interval_label_spinbox);

                auto btn_inc = lv_btn_create(other_settings_panel);
                lv_obj_set_size(btn_inc, spin_box_height, spin_box_height);

                lv_obj_set_style_bg_img_src(btn_inc, LV_SYMBOL_PLUS, 0);

                auto btn_dec = lv_btn_create(other_settings_panel);
                lv_obj_set_size(btn_dec, spin_box_height, spin_box_height);
                lv_obj_set_style_bg_img_src(btn_dec, LV_SYMBOL_MINUS, 0);

                add_event_callback(btn_inc, [this](lv_event_t *e)
                                   {
                    lv_event_code_t code = lv_event_get_code(e);
                    if(code == LV_EVENT_SHORT_CLICKED || code  == LV_EVENT_LONG_PRESSED_REPEAT) {
                        lv_spinbox_increment(ntp_server_refresh_interval_label_spinbox);
                    } });

                add_event_callback(btn_dec, [this](lv_event_t *e)
                                   {
                    lv_event_code_t code = lv_event_get_code(e);
                    if(code == LV_EVENT_SHORT_CLICKED || code  == LV_EVENT_LONG_PRESSED_REPEAT) {
                        lv_spinbox_decrement(ntp_server_refresh_interval_label_spinbox);
                    } });

                lv_obj_align_to(ntp_server_refresh_interval_label, last_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, y_pad);
                lv_obj_align_to(btn_inc, ntp_server_refresh_interval_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
                lv_obj_align_to(ntp_server_refresh_interval_label_spinbox, btn_inc, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
                lv_obj_align_to(btn_dec, ntp_server_refresh_interval_label_spinbox, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
                last_obj = btn_inc;
            }

            {
                //  brightness label
                auto brightness_panel_label = lv_label_create(other_settings_panel);
                lv_label_set_text(brightness_panel_label, "Screen Brightness");
                lv_obj_set_style_text_font(brightness_panel_label, lv_title_font, 0);

                //  brightness label switch label
                auto auto_brightness_switch_label = lv_label_create(other_settings_panel);
                lv_label_set_text(auto_brightness_switch_label, "Auto");

                auto auto_brightness_switch = lv_switch_create(other_settings_panel);

                // brightness slider
                settings_screen_tab_settings_brightness_slider = lv_slider_create(other_settings_panel);
                lv_obj_set_width(settings_screen_tab_settings_brightness_slider, lv_pct(66));
                lv_slider_set_range(settings_screen_tab_settings_brightness_slider, 1, 255);
                lv_obj_refresh_ext_draw_size(settings_screen_tab_settings_brightness_slider);

                lv_obj_align_to(brightness_panel_label, last_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, y_pad);
                lv_obj_align_to(auto_brightness_switch_label, brightness_panel_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 13);
                lv_obj_align_to(auto_brightness_switch, auto_brightness_switch_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
                lv_obj_align_to(settings_screen_tab_settings_brightness_slider, auto_brightness_switch, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

                add_event_callback(settings_screen_tab_settings_brightness_slider, [this](lv_event_t *e)
                                   {
                                       const lv_event_code_t code = lv_event_get_code(e);
                                       if (code == LV_EVENT_VALUE_CHANGED)
                                       {
                                           const auto value = lv_slider_get_value(settings_screen_tab_settings_brightness_slider);

                                           if (value != config::instance.data.get_manual_screen_brightness()) {
                                           ui_interface_instance.set_screen_brightness(value);
                                                config::instance.data.set_manual_screen_brightness(value);
                                                config::instance.save();
                                           }
                                       } });
                last_obj = auto_brightness_switch_label;
            }
        }
    }

    // Information tab
    {
        auto settings_screen_tab_information = lv_tabview_add_tab(settings_screen_tab, "Information");
        settings_screen_tab_information_table = lv_table_create(settings_screen_tab_information);
        lv_obj_set_size(settings_screen_tab_information_table, lv_pct(100), LV_SIZE_CONTENT);
    }

    create_close_button_to_main_screen(settings_screen_tab);
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
    font_montserrat_regular_numbers_48 = lv_font_load("S:display/font/montserrat/ui_font_m48regularnumbers.bin");
    log_d("2");
    font_montserrat_regular_numbers_40 = lv_font_load("S:display/font/montserrat/ui_font_m40regularnumbers.bin");
    log_d("3");
    font_montserrat_light_numbers_112 = lv_font_load("S:display/font/montserrat/ui_font_m112lightnumbers.bin");
    log_d("4");
    font_montserrat_medium_48 = lv_font_load("S:display/font/montserrat/ui_font_m48medium.bin");
    log_d("5");
    font_montserrat_medium_14 = lv_font_load("S:display/font/montserrat/ui_font_m14medium.bin");
    log_d("6");
    font_montserrat_medium_units_18 = lv_font_load("S:display/font/montserrat/ui_font_m18unitsmedium.bin");

    log_d("Loaded From SD Card");
}

void ui::init()
{
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_LIGHT_BLUE), lv_palette_main(LV_PALETTE_RED),
                                              false, LV_FONT_DEFAULT);

    lv_disp_set_theme(dispp, theme);
    boot_screen_screen_init();

    lv_disp_load_scr(boot_screen);

    inline_loop(100);

    log_i("Loaded boot screen");

    lv_label_set_text(boot_message, "Loading from SD Card");
    inline_loop(50);

    load_from_sd_card(); // might take some time

    main_screen_screen_init();
    sensor_detail_screen_init();
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

void ui::set_label_panel_color(lv_obj_t *panel, uint8_t level)
{
    uint32_t color;
    uint32_t color_grad;

    switch (level)
    {
    default:
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
    case 5:
        color = 0x940606; // Maroon
        color_grad = 0xc30808;
        break;
    case no_value_label:
        color = 0xC0C0C0; // Silver
        color_grad = 0x696969;
        break;
    }

    lv_obj_set_style_bg_color(panel, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(panel, lv_color_hex(color_grad), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void ui::set_value_in_panel(const panel_and_label &pair, sensor_id_index index, const std::optional<sensor_value::value_type> &value)
{
    if (value.has_value())
    {
        const auto final_value = value.value();
        const auto level = sensor_definitions[static_cast<uint8_t>(index)].calculate_level(final_value);
        if (pair.panel)
        {
            set_label_panel_color(pair.panel, level);
        }

        if (pair.label)
        {
            if (!pair.panel)
            {
                lv_label_set_text_fmt(pair.label, "%d%s", final_value, sensor_definitions[static_cast<uint8_t>(index)].get_unit());
            }
            else
            {
                lv_label_set_text_fmt(pair.label, "%d", final_value);
            }
        }
    }
    else
    {
        set_default_value_in_panel(pair);
    }
}

void ui::set_default_value_in_panel(const panel_and_label &pair)
{
    if (pair.panel)
    {
        set_label_panel_color(pair.panel, no_value_label);
    }

    if (pair.label)
    {
        lv_label_set_text_static(pair.label, "-");
    }
}

void ui::set_sensor_value(sensor_id_index index, const std::optional<sensor_value::value_type> &value)
{
    const auto active_screen = lv_scr_act();

    if (active_screen == main_screen)
    {
        log_d("Updating sensor %d to %d in main screen", index, value.value_or(-1));
        const auto &pair = main_screen_panel_and_label.at(static_cast<size_t>(index));
        set_value_in_panel(pair, index, value);
    }
    else if (active_screen == sensor_detail_screen)
    {
        if (lv_obj_get_user_data(sensor_detail_screen) == reinterpret_cast<void *>(index))
        {
            log_d("Updating sensor %d to %d in details screen", index, value.value_or(-1));
            detail_screen_current_values(index, value);
        }
    }
}

void ui::update_configuration()
{
    lv_textarea_set_text(host_name_text_area, config::instance.data.get_host_name().c_str());
    lv_textarea_set_text(ntp_server_text_area, config::instance.data.get_ntp_server().c_str());
    lv_spinbox_set_value(ntp_server_refresh_interval_label_spinbox, config::instance.data.get_ntp_server_refresh_interval() / 1000);
    lv_slider_set_value(settings_screen_tab_settings_brightness_slider, config::instance.data.get_manual_screen_brightness().value_or(0), LV_ANIM_OFF);
}

void ui::update_boot_message(const String &message)
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

struct _lv_event_dsc_t *ui::add_event_callback(lv_obj_t *obj, std::function<void(lv_event_t *)> ftn, lv_event_code_t filter)
{
    auto param = new std::function<void(lv_event_t *)>(ftn);
    return lv_obj_add_event_cb(obj, event_callback_ftn, LV_EVENT_ALL, param);
}

void ui::add_panel_callback_event(lv_obj_t *panel, sensor_id_index index)
{
    add_event_callback(
        panel, [this, index](lv_event_t *e)
        { if (e->code == LV_EVENT_CLICKED) {
            show_sensor_detail_screen(index);
        } },
        LV_EVENT_PRESSED);
}

void ui::detail_screen_current_values(sensor_id_index index, const std::optional<sensor_value::value_type> &value)
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

void ui::show_sensor_detail_screen(sensor_id_index index)
{
    log_i("Panel pressed for sensor index:%d", index);

    lv_obj_set_user_data(sensor_detail_screen, reinterpret_cast<void *>(index));

    lv_label_set_text(sensor_detail_screen_top_label, sensor_definitions[static_cast<uint8_t>(index)].get_name());
    lv_label_set_text_static(sensor_detail_screen_top_label_units, sensor_definitions[static_cast<uint8_t>(index)].get_unit());

    const auto value = ui_interface_instance.get_sensor_value(index);
    detail_screen_current_values(index, value);

    lv_scr_load_anim(sensor_detail_screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
}