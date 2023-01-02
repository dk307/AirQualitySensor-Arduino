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

        wifi_setting_image = lv_img_create(screen);
        lv_img_set_src(wifi_setting_image, "S:display/image/wifi.png");
        lv_obj_add_flag(wifi_setting_image, LV_OBJ_FLAG_HIDDEN);

        const lv_font_t *lv_title_font = &lv_font_montserrat_16;

        auto settings_screen_tab = lv_tabview_create(screen, LV_DIR_LEFT, 64);
        lv_obj_set_style_text_font(screen, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_add_event_cb(screen, event_callback<ui_settings_screen, &ui_settings_screen::screen_events_callback>, LV_EVENT_ALL, this);

        // Wifi tab
        {
            auto tab_wifi = lv_tabview_add_tab(settings_screen_tab, LV_SYMBOL_WIFI);

            const int y_pad = 15;

            auto panel = lv_obj_create(tab_wifi);
            lv_obj_set_size(panel, lv_pct(100), LV_SIZE_CONTENT);

            auto wifi_credential_label = lv_label_create(panel);
            lv_label_set_text_static(wifi_credential_label, "Wifi Network:");
            lv_obj_set_style_text_font(wifi_credential_label, lv_title_font, LV_PART_MAIN | LV_STATE_DEFAULT);

            auto wifi_network = lv_label_create(panel);
            lv_obj_set_style_text_font(wifi_credential_label, lv_title_font, LV_PART_MAIN | LV_STATE_DEFAULT);

            auto wifi_credential_button = lv_btn_create(panel);
            lv_obj_set_style_bg_img_src(wifi_credential_button, LV_SYMBOL_EDIT, 0);
            lv_obj_set_size(wifi_credential_button, 15, 15);         

            lv_obj_align_to(wifi_network, wifi_credential_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
            lv_obj_align_to(wifi_credential_button, wifi_network, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
        }

        // Homekit tab
        {
            auto settings_screen_tab_information = lv_tabview_add_tab(settings_screen_tab, LV_SYMBOL_CALL);
        }

        // Settings tab
        {
            auto tab_settings = lv_tabview_add_tab(settings_screen_tab, LV_SYMBOL_SETTINGS);

            settings_screen_tab_settings_kb = lv_keyboard_create(screen);
            lv_obj_set_size(settings_screen_tab_settings_kb, screen_width, screen_height / 2);
            lv_obj_set_style_text_font(settings_screen_tab_settings_kb, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_flag(settings_screen_tab_settings_kb, LV_OBJ_FLAG_HIDDEN);

            // Settings - other panel
            {
                const int y_pad = 15;

                auto panel = lv_obj_create(tab_settings);
                lv_obj_set_size(panel, lv_pct(100), LV_SIZE_CONTENT);

                lv_obj_t *last_obj = nullptr;
                {
                    // hostname label
                    auto host_name_text_area_label = lv_label_create(panel);
                    lv_label_set_text_static(host_name_text_area_label, "Hostname:");

                    // hostname text area
                    host_name_text_area = lv_textarea_create(panel);
                    lv_textarea_set_one_line(host_name_text_area, true);
                    lv_obj_set_width(host_name_text_area, lv_pct(100));

                    lv_obj_set_style_text_font(host_name_text_area_label, lv_title_font, LV_PART_MAIN | LV_STATE_DEFAULT);

                    add_event_callback(host_name_text_area, [this](lv_event_t *e)
                                       {
                    if (settings_key_board_event_cb(e)) {
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
                    auto ntp_server_text_area_label = lv_label_create(panel);
                    lv_label_set_text_static(ntp_server_text_area_label, "NTP Server:");
                    lv_obj_set_style_text_font(ntp_server_text_area_label, lv_title_font, LV_PART_MAIN | LV_STATE_DEFAULT);

                    // ntp server text area
                    ntp_server_text_area = lv_textarea_create(panel);
                    lv_textarea_set_one_line(ntp_server_text_area, true);
                    lv_obj_set_width(ntp_server_text_area, lv_pct(100));

                    add_event_callback(ntp_server_text_area, [this](lv_event_t *e)
                                       {
                    if (settings_key_board_event_cb(e)) {
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
                    auto ntp_server_refresh_interval_label = lv_label_create(panel);
                    lv_label_set_text_static(ntp_server_refresh_interval_label, "NTP Server sync interval (seconds):");
                    lv_obj_set_style_text_font(ntp_server_refresh_interval_label, lv_title_font, LV_PART_MAIN | LV_STATE_DEFAULT);

                    // ntp server refresh interval spin box
                    ntp_server_refresh_interval_label_spinbox = lv_spinbox_create(panel);
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

                    auto btn_inc = lv_btn_create(panel);
                    lv_obj_set_size(btn_inc, spin_box_height, spin_box_height);

                    lv_obj_set_style_bg_img_src(btn_inc, LV_SYMBOL_PLUS, 0);

                    auto btn_dec = lv_btn_create(panel);
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
                    auto timezone_label = lv_label_create(panel);
                    lv_label_set_text_static(timezone_label, "Timezone");
                    lv_obj_set_style_text_font(timezone_label, lv_title_font, 0);

                    timezone_drop_down = lv_roller_create(panel);
                    lv_roller_set_options(timezone_drop_down, "USA Eastern\n"
                                                              "USA Central\n"
                                                              "USA Mountain time\n"
                                                              "USA Arizona\n"
                                                              "USA Pacific",
                                          LV_ROLLER_MODE_NORMAL);
                    lv_roller_set_visible_row_count(timezone_drop_down, 3);
                    lv_obj_set_width(timezone_drop_down, lv_pct(100));

                    add_event_callback(
                        timezone_drop_down, [this](lv_event_t *e)
                        {
                                       const lv_event_code_t code = lv_event_get_code(e);
                                       if (code == LV_EVENT_VALUE_CHANGED)
                                       {
                                           const auto value = 
                                           static_cast<TimeZoneSupported>(lv_roller_get_selected(timezone_drop_down));

                                           if (value != config::instance.data.get_timezone()) {               
                                                config::instance.data.set_timezone(value);
                                                config::instance.save();
                                           }
                                       } },
                        LV_EVENT_VALUE_CHANGED);

                    lv_obj_align_to(timezone_label, last_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, y_pad);
                    lv_obj_align_to(timezone_drop_down, timezone_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
                    last_obj = timezone_drop_down;
                }

                {
                    //  brightness label
                    auto brightness_panel_label = lv_label_create(panel);
                    lv_label_set_text_static(brightness_panel_label, "Screen Brightness");
                    lv_obj_set_style_text_font(brightness_panel_label, lv_title_font, 0);

                    //  brightness label switch label
                    auto auto_brightness_switch_label = lv_label_create(panel);
                    lv_label_set_text(auto_brightness_switch_label, "Auto");

                    auto auto_brightness_switch = lv_switch_create(panel);

                    // brightness slider
                    brightness_slider = lv_slider_create(panel);
                    lv_obj_set_width(brightness_slider, lv_pct(66));
                    lv_slider_set_range(brightness_slider, 1, 255);
                    lv_obj_refresh_ext_draw_size(brightness_slider);

                    lv_obj_align_to(brightness_panel_label, last_obj, LV_ALIGN_OUT_BOTTOM_LEFT, 0, y_pad);
                    lv_obj_align_to(auto_brightness_switch_label, brightness_panel_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 13);
                    lv_obj_align_to(auto_brightness_switch, auto_brightness_switch_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
                    lv_obj_align_to(brightness_slider, auto_brightness_switch, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

                    add_event_callback(
                        brightness_slider, [this](lv_event_t *e)
                        {
                                       const lv_event_code_t code = lv_event_get_code(e);
                                       if (code == LV_EVENT_VALUE_CHANGED)
                                       {
                                           const auto value = lv_slider_get_value(brightness_slider);

                                           if (value != config::instance.data.get_manual_screen_brightness()) {
                                           ui_interface_instance.set_screen_brightness(value);
                                                config::instance.data.set_manual_screen_brightness(value);
                                                config::instance.save();
                                           }
                                       } },
                        LV_EVENT_VALUE_CHANGED);
                    last_obj = auto_brightness_switch_label;
                }
            }
        }

        // Information tab
        {
            auto settings_screen_tab_information = lv_tabview_add_tab(settings_screen_tab, LV_SYMBOL_LIST);
            tab_information_table = lv_table_create(settings_screen_tab_information);
            lv_obj_set_size(tab_information_table, lv_pct(100), LV_SIZE_CONTENT);
        }

        auto settings_screen_tab_btns = lv_tabview_get_tab_btns(settings_screen_tab);
        // lv_obj_add_event_cb(settings_screen_tab_btns,
        //                     event_callback<ui_settings_screen, &ui_settings_screen::settings_screen_tab_btns_event_cb>,
        //                     LV_EVENT_ALL, this);
        lv_obj_set_style_text_font(settings_screen_tab_btns, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

        create_close_button_to_main_screen(screen, LV_ALIGN_TOP_RIGHT, -15, 15);
    }

    void update_configuration()
    {
        lv_textarea_set_text(host_name_text_area, config::instance.data.get_host_name().c_str());
        lv_textarea_set_text(ntp_server_text_area, config::instance.data.get_ntp_server().c_str());
        lv_spinbox_set_value(ntp_server_refresh_interval_label_spinbox, config::instance.data.get_ntp_server_refresh_interval() / 1000);
        lv_dropdown_set_selected(timezone_drop_down, static_cast<uint16_t>(config::instance.data.get_timezone()));
        lv_slider_set_value(brightness_slider, config::instance.data.get_manual_screen_brightness().value_or(0), LV_ANIM_OFF);

        // const auto ssid = config::instance.data.get_wifi_ssid();
        // lv_label_set_text(wifi_network, ssid.isEmpty() ? "None Set" : ssid.c_str());
    }

    void settings_screen_tab_btns_event_cb(lv_event_t *e)
    {
        lv_event_code_t code = lv_event_get_code(e);

        if (code == LV_EVENT_DRAW_PART_BEGIN)
        {
            lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
            if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN)
            {
                dsc->label_dsc->opa = LV_OPA_TRANSP; /*Hide the text if any*/
            }
        }
        else if (code == LV_EVENT_DRAW_PART_END)
        {
            lv_obj_t *obj = lv_event_get_target(e);

            lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);

            if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN)
            {
                log_d("%d", dsc->id);
                {
                    const auto w = lv_obj_get_width(wifi_setting_image);
                    const auto h = lv_obj_get_height(wifi_setting_image);

                    lv_area_t area;
                    area.x1 = dsc->draw_area->x1 + (lv_area_get_width(dsc->draw_area) - w) / 2;
                    area.x2 = area.x1 + w - 1;
                    area.y1 = dsc->draw_area->y1 + (lv_area_get_height(dsc->draw_area) - h) / 2;
                    area.y2 = area.y1 + h - 1;

                    lv_draw_img_dsc_t img_draw_dsc;
                    lv_draw_img_dsc_init(&img_draw_dsc);
                    img_draw_dsc.recolor = lv_color_black();
                    if (lv_btnmatrix_get_selected_btn(obj) == dsc->id)
                    {
                        img_draw_dsc.recolor_opa = LV_OPA_30;
                    }

                    lv_draw_img(dsc->draw_ctx, &img_draw_dsc, &area, lv_img_get_src(wifi_setting_image));
                }
            }
        }
    }

    void show_screen()
    {
        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    }

private:
    lv_obj_t *wifi_network;

    lv_obj_t *settings_screen_tab_settings_kb;
    lv_obj_t *tab_information_table;
    lv_obj_t *host_name_text_area;
    lv_obj_t *ntp_server_text_area;
    lv_obj_t *ntp_server_refresh_interval_label_spinbox;
    lv_obj_t *timezone_drop_down;
    lv_obj_t *brightness_slider;

    lv_obj_t *wifi_setting_image;

    lv_timer_t *information_refresh_timer;

    bool settings_key_board_event_cb(lv_event_t *e)
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

    void screen_events_callback(lv_event_t *e)
    {
        lv_event_code_t event_code = lv_event_get_code(e);
        if (event_code == LV_EVENT_SCREEN_LOAD_START)
        {
            log_d("setting screen shown");
            load_information(nullptr);
            update_configuration();
            information_refresh_timer = lv_timer_create(timer_callback<ui_settings_screen, &ui_settings_screen::load_information>, 1000, this);
        }
        else if (event_code == LV_EVENT_SCREEN_UNLOADED)
        {
            log_d("setting screen hidden");
            if (information_refresh_timer)
            {
                lv_timer_del(information_refresh_timer);
                information_refresh_timer = nullptr;
            }
        }
    }

    void load_information(lv_timer_t *)
    {
        log_v("updating info table");
        const auto data = ui_interface_instance.get_information_table();

        lv_table_set_col_cnt(tab_information_table, 2);
        lv_table_set_row_cnt(tab_information_table, data.size());

        lv_table_set_col_width(tab_information_table, 0, 140);
        lv_table_set_col_width(tab_information_table, 1, 430 - 140);

        for (auto i = 0; i < data.size(); i++)
        {
            lv_table_set_cell_value(tab_information_table, i, 0, std::get<0>(data[i]).c_str());
            lv_table_set_cell_value(tab_information_table, i, 1, std::get<1>(data[i]).c_str());
        }
    }
};