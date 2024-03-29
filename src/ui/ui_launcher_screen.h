#pragma once

#include "ui_screen.h"
#include "config_manager.h"
#include "operations.h"

#include <task_wrapper.h>

class ui_launcher_screen : public ui_screen
{
public:
    using ui_screen::ui_screen;
    void init() override
    {
        ui_screen::init();
        set_default_screen();

        lv_obj_add_event_cb(screen, event_callback<ui_launcher_screen, &ui_launcher_screen::screen_callback>, LV_EVENT_ALL, this);

        lv_style_init(&button_style_default);
        lv_style_set_text_color(&button_style_default, text_color);
        lv_style_set_img_opa(&button_style_default, LV_OPA_100);
        lv_style_set_text_font(&button_style_default, &lv_font_montserrat_16);

        lv_style_init(&button_style_pressed);
        lv_style_set_img_recolor_opa(&button_style_pressed, LV_OPA_30);
        lv_style_set_img_recolor(&button_style_pressed,  text_color);
        lv_style_set_text_color(&button_style_pressed, lv_color_lighten(text_color, LV_OPA_50));
        lv_style_set_text_font(&button_style_pressed, &lv_font_montserrat_16);

        int pad = 30;
        create_button(LV_ALIGN_TOP_LEFT, pad, pad,
                      "S:display/image/restart.png", "Restart",
                      event_callback<ui_launcher_screen, &ui_launcher_screen::restart>);
        create_button(LV_ALIGN_TOP_MID, 0, pad,
                      "S:display/image/factory-reset.png", "Factory\nReset",
                      event_callback<ui_launcher_screen, &ui_launcher_screen::factory_reset>);
        create_button(LV_ALIGN_TOP_RIGHT, -pad, pad,
                      "S:display/image/info.png", "Information",
                      event_callback<ui_launcher_screen, &ui_launcher_screen::show_information>);
        create_button(LV_ALIGN_BOTTOM_MID, 0, -pad - 25,
                      "S:display/image/info.png", "Hardware",
                      event_callback<ui_launcher_screen, &ui_launcher_screen::hardware_info>);

        init_confirm_win();

        create_close_button_to_main_screen(screen, LV_ALIGN_BOTTOM_LEFT, 15, -15);
    }

    void show_screen()
    {
        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }

private:
    lv_style_t button_style_default;
    lv_style_t button_style_pressed;
    lv_obj_t *win_confirm;
    lv_obj_t *win_confirm_label;

    void screen_callback(lv_event_t *e)
    {
        lv_event_code_t event_code = lv_event_get_code(e);

        if ((event_code == LV_EVENT_SCREEN_LOAD_START) || (event_code == LV_EVENT_SCREEN_UNLOADED))
        {
            lv_obj_add_flag(win_confirm, LV_OBJ_FLAG_HIDDEN);
        }
    }

    lv_obj_t *create_button(lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs,
                            const char *src, const char *label_str,
                            lv_event_cb_t event_cb)
    {
        auto btn = lv_imgbtn_create(screen);
        lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_align(btn, align, x_ofs, y_ofs);
        lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, src, NULL);

        lv_obj_add_style(btn, &button_style_default, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(btn, &button_style_pressed, LV_STATE_PRESSED);

        auto label = lv_label_create(screen);
        lv_label_set_text_static(label, label_str);
        lv_obj_align_to(label, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);

        lv_obj_add_style(label, &button_style_default, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_style(label, &button_style_pressed, LV_STATE_PRESSED);

        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(label, event_cb, LV_EVENT_CLICKED, this);

        return btn;
    }

    void init_confirm_win()
    {
        win_confirm = lv_win_create(screen, 35);
        lv_win_add_title(win_confirm, "Confirm");
        lv_obj_set_size(win_confirm, 2 * screen_width / 3, screen_height / 2);

        auto cont = lv_win_get_content(win_confirm);
        win_confirm_label = lv_label_create(cont);
        lv_label_set_recolor(win_confirm_label, true);

        auto btn_yes = lv_btn_create(cont);
        auto btn_yes_label = lv_label_create(btn_yes);
        lv_label_set_text_static(btn_yes_label, "Yes");
        lv_obj_center(btn_yes_label);

        auto btn_no = lv_btn_create(cont);
        auto btn_no_label = lv_label_create(btn_no);
        lv_label_set_text_static(btn_no_label, "No");
        lv_obj_center(btn_no_label);

        lv_obj_add_event_cb(btn_yes, event_callback<ui_launcher_screen, &ui_launcher_screen::yes_win_confirm>, LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(btn_no, event_callback<ui_launcher_screen, &ui_launcher_screen::close_win_confirm>, LV_EVENT_CLICKED, this);

        lv_obj_center(win_confirm);
        lv_obj_align(win_confirm_label, LV_ALIGN_TOP_MID, 0, 5);
        lv_obj_align(btn_yes, LV_ALIGN_BOTTOM_LEFT, 5, -5);
        lv_obj_align(btn_no, LV_ALIGN_BOTTOM_RIGHT, -5, -5);

        lv_obj_add_flag(win_confirm, LV_OBJ_FLAG_HIDDEN);
    }

    void show_information(lv_event_t *e)
    {
        ESP_LOGI(UI_TAG, "Showing information clicked");
        inter_screen_interface.show_setting_screen();
    }

    void hardware_info(lv_event_t *e)
    {
        ESP_LOGI(UI_TAG, "Showing Hardware clicked");
        inter_screen_interface.show_hardware_info_screen();
    }

    void restart(lv_event_t *)
    {
        ESP_LOGI(UI_TAG, "Restart clicked");
        show_confirm(confirm_type::restart);
    }

    void factory_reset(lv_event_t *)
    {
        ESP_LOGI(UI_TAG, "Factory Reset");
        show_confirm(confirm_type::factory_reset);
    }

    void yes_win_confirm(lv_event_t *e)
    {
        ESP_LOGI(UI_TAG, "Yes clicked");
        lv_obj_add_flag(win_confirm, LV_OBJ_FLAG_HIDDEN);
        confirm_type type = (confirm_type)(uintptr_t)lv_obj_get_user_data(win_confirm);

        switch (type)
        {
        case confirm_type::restart:
            inter_screen_interface.show_top_level_message("Restarting", 60000);
            operations::instance.reboot();
            break;
        case confirm_type::factory_reset:
            inter_screen_interface.show_top_level_message("Factory Reseting", 60000);
            operations::instance.factory_reset();
            break;
        }
    }

    void close_win_confirm(lv_event_t *e)
    {
        ESP_LOGI(UI_TAG, "No clicked");
        lv_obj_add_flag(win_confirm, LV_OBJ_FLAG_HIDDEN);
    }

    enum class confirm_type
    {
        restart,
        factory_reset
    };

    void show_confirm(confirm_type type)
    {
        lv_obj_set_user_data(win_confirm, (void *)type);
        switch (type)
        {
        case confirm_type::restart:
            lv_label_set_text_static(win_confirm_label, "Do you want to restart?");
            break;
        case confirm_type::factory_reset:
            lv_label_set_text_static(win_confirm_label, "#ff0000 Do you want to factory reset?#");
            break;
        }
        lv_obj_clear_flag(win_confirm, LV_OBJ_FLAG_HIDDEN);
    }
};