#pragma once

#include "ui_screen.h"
#include "config_manager.h"

#include <task_wrapper.h>

class ui_settings_screen : public ui_screen
{
public:
    using ui_screen::ui_screen;
    void init() override
    {
        ui_screen::init();

        const auto lv_title_font = &lv_font_montserrat_16;

        auto settings_screen_tab = lv_tabview_create(screen, LV_DIR_LEFT, 80);
        lv_obj_set_style_text_font(screen, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

        auto settings_screen_tab_btns = lv_tabview_get_tab_btns(settings_screen_tab);

        add_event_callback(settings_screen_tab_btns, [this](lv_event_t *e)
                           {

         lv_event_code_t code = lv_event_get_code(e);
         lv_obj_t * obj = lv_event_get_target(e);
 
       lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);

   
      if(dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
           if(dsc->id == 3) {

             
    //             lv_img_header_t header;
    //             lv_res_t res = lv_img_decoder_get_info(&img_star, &header);
    //             if(res != LV_RES_OK) return;

    //             lv_area_t a;
    //             a.x1 = dsc->draw_area->x1 + (lv_area_get_width(dsc->draw_area) - header.w) / 2;
    //             a.x2 = a.x1 + header.w - 1;
    //             a.y1 = dsc->draw_area->y1 + (lv_area_get_height(dsc->draw_area) - header.h) / 2;
    //             a.y2 = a.y1 + header.h - 1;

    //             lv_draw_img_dsc_t img_draw_dsc;
    //             lv_draw_img_dsc_init(&img_draw_dsc);
    //             img_draw_dsc.recolor = lv_color_black();
    //             if(lv_btnmatrix_get_selected_btn(obj) == dsc->id)  img_draw_dsc.recolor_opa = LV_OPA_30;

                 //  lv_draw_img(dsc->draw_ctx, &img_draw_dsc, &a, &img_star);
            }
        
                   } });

        lv_obj_add_event_cb(screen, event_callback<ui_settings_screen, &ui_settings_screen::settings_screen_events_callback>, LV_EVENT_ALL, this);

        // Wifi tab
        {
            auto settings_screen_tab_information = lv_tabview_add_tab(settings_screen_tab, "Wifi");
        }

        // Homekit tab
        {
            auto settings_screen_tab_information = lv_tabview_add_tab(settings_screen_tab, "Homekit");
        }

        // Settings tab
        {
            auto settings_screen_tab_settings = lv_tabview_add_tab(settings_screen_tab, "Settings");

            settings_screen_tab_settings_kb = lv_keyboard_create(screen);
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

        create_close_button_to_main_screen(screen, LV_ALIGN_TOP_RIGHT, -15, 15);
    }

    void update_configuration()
    {
        lv_textarea_set_text(host_name_text_area, config::instance.data.get_host_name().c_str());
        lv_textarea_set_text(ntp_server_text_area, config::instance.data.get_ntp_server().c_str());
        lv_spinbox_set_value(ntp_server_refresh_interval_label_spinbox, config::instance.data.get_ntp_server_refresh_interval() / 1000);
        lv_slider_set_value(settings_screen_tab_settings_brightness_slider, config::instance.data.get_manual_screen_brightness().value_or(0), LV_ANIM_OFF);
    }

    void show_screen()
    {
        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }

private:
    // settings screen
    lv_obj_t *settings_screen_tab_settings_kb;
    lv_obj_t *settings_screen_tab_information_table;
    lv_obj_t *host_name_text_area;
    lv_obj_t *ntp_server_text_area;
    lv_obj_t *ntp_server_refresh_interval_label_spinbox;
    lv_obj_t *settings_screen_tab_settings_brightness_slider;

    std::unique_ptr<task_wrapper> information_refresh_task;

    bool settings_screen_screen_key_board_event_cb(lv_event_t *e)
    {
        const lv_event_code_t code = lv_event_get_code(e);
        lv_obj_t *ta = lv_event_get_target(e);

        if ((code == LV_EVENT_FOCUSED) || (code == LV_EVENT_SHORT_CLICKED))
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

    void settings_screen_events_callback(lv_event_t *e)
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

    void load_information()
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
};