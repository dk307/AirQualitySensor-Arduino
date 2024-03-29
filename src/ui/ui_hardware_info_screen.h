#pragma once

#include "ui_screen.h"
#include "config_manager.h"
#include "operations.h"

class ui_hardware_info_screen : public ui_screen
{
public:
    using ui_screen::ui_screen;
    void init() override
    {
        ui_screen::init();
        set_default_screen();

        lv_obj_add_event_cb(screen, event_callback<ui_hardware_info_screen, &ui_hardware_info_screen::screen_callback>, LV_EVENT_ALL, this);

        // baseline
        auto btn_clean = create_btn("Clean SPS 30",
                                    event_callback<ui_hardware_info_screen, &ui_hardware_info_screen::clean_sps_30>);

        lv_obj_align(btn_clean, LV_ALIGN_TOP_MID, 0, 25);

        init_status_win();

        create_close_button_to_main_screen(screen, LV_ALIGN_BOTTOM_LEFT, 15, -15);
    }

    void show_screen()
    {
        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }

private:
    lv_obj_t *win_status;
    lv_obj_t *win_status_label;

    void screen_callback(lv_event_t *e)
    {
        lv_event_code_t event_code = lv_event_get_code(e);

        if ((event_code == LV_EVENT_SCREEN_LOAD_START) || (event_code == LV_EVENT_SCREEN_UNLOADED))
        {
            lv_obj_add_flag(win_status, LV_OBJ_FLAG_HIDDEN);
        }
    }

    lv_obj_t *create_btn(const char *label_text, lv_event_cb_t event_cb)
    {
        auto btn_set_baseline = lv_btn_create(screen);
        lv_obj_add_event_cb(btn_set_baseline, event_cb, LV_EVENT_ALL, this);
        lv_obj_set_height(btn_set_baseline, LV_SIZE_CONTENT);

        auto label = lv_label_create(btn_set_baseline);
        lv_label_set_text_static(label, label_text);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_center(label);

        return btn_set_baseline;
    }

    void init_status_win()
    {
        win_status = lv_win_create(screen, 35);
        lv_win_add_title(win_status, "Status");
        lv_obj_set_size(win_status, 2 * screen_width / 3, screen_height / 2);

        auto cont = lv_win_get_content(win_status);
        win_status_label = lv_label_create(cont);

        auto btn_close = lv_btn_create(cont);
        auto btn_close_label = lv_label_create(btn_close);
        lv_label_set_text_static(btn_close_label, "Close");
        lv_obj_center(btn_close_label);

        lv_obj_add_event_cb(btn_close,
                            event_callback<ui_hardware_info_screen, &ui_hardware_info_screen::close_win_confirm>,
                            LV_EVENT_CLICKED, this);

        lv_obj_center(win_status);
        lv_obj_align(win_status_label, LV_ALIGN_TOP_MID, 0, 5);
        lv_obj_align(btn_close, LV_ALIGN_BOTTOM_MID, 0, -5);

        lv_obj_add_flag(win_status, LV_OBJ_FLAG_HIDDEN);
    }

    void close_win_confirm(lv_event_t *e)
    {
        ESP_LOGI(UI_TAG, "No clicked");
        lv_obj_add_flag(win_status, LV_OBJ_FLAG_HIDDEN);
    }

    void clean_sps_30(lv_event_t *e)
    {
        const auto code = lv_event_get_code(e);

        if (code == LV_EVENT_PRESSED)
        {
            const bool success = ui_interface_instance.clean_sps_30();
            lv_label_set_text_static(win_status_label, success ? "Cleanup started." : "Clean start failed.");
            lv_obj_clear_flag(win_status, LV_OBJ_FLAG_HIDDEN);
        }
    }
};