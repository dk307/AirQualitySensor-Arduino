#include "ui2.h"
#include "ui_interface.h"
#include <task_wrapper.h>

#include <tuple>
#include <memory>

///////////////////// VARIABLES ////////////////////
lv_obj_t *ui_bootscreen;
lv_obj_t *ui_bootlogo;
lv_obj_t *ui_boot_message;
lv_obj_t *ui_main_screen;
lv_obj_t *ui_settings_screen;
lv_obj_t *ui_aqi_value_label;
lv_obj_t *ui_settings_screen_tab_information_table;
lv_obj_t *ui_settings_screen_tab_settings_brightness_slider;

static const lv_font_t *font_large = &lv_font_montserrat_20;
static const lv_font_t *font_normal = &lv_font_montserrat_14;
static const lv_font_t *font_extra_large_number = &lv_font_montserrat_20;

static EXT_RAM_ATTR lv_style_t style_text_muted;
static EXT_RAM_ATTR lv_style_t style_title;
static EXT_RAM_ATTR lv_style_t style_label_default;

static std::unique_ptr<task_wrapper> information_refresh_task;

LV_IMG_DECLARE(ui_img_1508142627); // assets\icons8-wind-100.png

///////////////////// FUNCTIONS ////////////////////
void ui_event_mainscreen(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);
    if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM)
    {
        lv_scr_load_anim(ui_settings_screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
    }
}

///////////////////// SCREENS ////////////////////
void ui_bootscreen_screen_init(void)
{
    ui_bootscreen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_bootscreen, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(ui_bootscreen, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(ui_bootscreen, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_bootscreen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_bootlogo = lv_img_create(ui_bootscreen);
    lv_img_set_src(ui_bootlogo, &ui_img_1508142627);
    lv_obj_set_size(ui_bootlogo, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(ui_bootlogo, LV_ALIGN_CENTER, 0, -20);

    ui_boot_message = lv_label_create(ui_bootscreen);
    lv_obj_set_size(ui_boot_message, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(ui_boot_message, LV_ALIGN_CENTER, 0, 60);
    lv_label_set_text(ui_boot_message, "Starting");
    lv_obj_set_style_text_color(ui_boot_message, lv_color_hex(0xFCFEFC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_boot_message, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void ui_main_screen_screen_init(void)
{
    ui_main_screen = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_main_screen, LV_OBJ_FLAG_SCROLLABLE);

    // AQI panel
    {
        auto ui_main_screen_aqi_panel = lv_obj_create(ui_main_screen);
        lv_obj_set_size(ui_main_screen_aqi_panel, lv_pct(50), lv_pct(100));
        lv_obj_set_style_border_width(ui_main_screen_aqi_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_left(ui_main_screen_aqi_panel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(ui_main_screen_aqi_panel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(ui_main_screen_aqi_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(ui_main_screen_aqi_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        const uint8_t extra_y = 40;
        auto ui_main_screen_Label1 = lv_label_create(ui_main_screen_aqi_panel);
        lv_obj_set_size(ui_main_screen_Label1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_align(ui_main_screen_Label1, LV_ALIGN_TOP_MID, 0, extra_y);
        lv_label_set_text(ui_main_screen_Label1, "AQI");
        lv_obj_set_style_text_color(ui_main_screen_Label1, lv_color_hex(0x1E1E1E), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_opa(ui_main_screen_Label1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui_main_screen_Label1, font_large, LV_PART_MAIN | LV_STATE_DEFAULT);

        ui_aqi_value_label = lv_label_create(ui_main_screen_aqi_panel);
        lv_obj_set_size(ui_aqi_value_label, lv_pct(100), LV_SIZE_CONTENT);
        lv_obj_align(ui_aqi_value_label, LV_ALIGN_TOP_MID, 0, extra_y + 20);
        lv_label_set_long_mode(ui_aqi_value_label, LV_LABEL_LONG_SCROLL);
        lv_obj_set_style_text_align(ui_aqi_value_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(ui_aqi_value_label, font_extra_large_number, LV_PART_MAIN | LV_STATE_DEFAULT);
        ui_set_aqi_value(999);

        lv_obj_add_event_cb(ui_main_screen, ui_event_mainscreen, LV_EVENT_ALL, NULL);
    }
}

void ui_load_information()
{
    log_d("updating info table");
    const auto data = ui_interface::instance.get_information_table();

    lv_table_set_col_cnt(ui_settings_screen_tab_information_table, 2);
    lv_table_set_row_cnt(ui_settings_screen_tab_information_table, data.size());

    lv_table_set_col_width(ui_settings_screen_tab_information_table, 0, 140);
    lv_table_set_col_width(ui_settings_screen_tab_information_table, 1, 430 - 140);

    for (auto i = 0; i < data.size(); i++)
    {
        lv_table_set_cell_value(ui_settings_screen_tab_information_table, i, 0, std::get<0>(data[i]).c_str());
        lv_table_set_cell_value(ui_settings_screen_tab_information_table, i, 1, std::get<1>(data[i]).c_str());
    }

    lv_slider_set_value(ui_settings_screen_tab_settings_brightness_slider, ui_interface::instance.get_manual_screen_brightness(), LV_ANIM_OFF);
}

void ui_settings_screen_events_callback(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_SCREEN_LOAD_START)
    {
        log_d("setting screen shown");
        ui_load_information();
        information_refresh_task = std::make_unique<task_wrapper>([]
                                                                  {
                                                                      for (;;)
                                                                      {
                                                                          ui_load_information();
                                                                          vTaskDelay(1000);
                                                                      } });

        information_refresh_task->spawn_arduino_main_core("ui info table refresh");
    }
    else if (event_code == LV_EVENT_SCREEN_UNLOADED)
    {
        log_d("setting screen hidden");
        information_refresh_task.reset();
    }
    else if (event_code == LV_EVENT_GESTURE && lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_BOTTOM)
    {
        lv_scr_load_anim(ui_main_screen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, false);
    }
}

void ui_settings_screen_tab_settings_brightness_slider_event_cb(lv_event_t *e)
{
    lv_event_code_t event_code = lv_event_get_code(e);
    if (event_code == LV_EVENT_VALUE_CHANGED)
    {
        const auto value = lv_slider_get_value(ui_settings_screen_tab_settings_brightness_slider);
        ui_interface::instance.set_manual_screen_brightness(value);
    }
}

void ui_settings_screen_screen_init(void)
{
    ui_settings_screen = lv_obj_create(NULL);

    auto ui_settings_screen_tab = lv_tabview_create(ui_settings_screen, LV_DIR_TOP, 45);
    lv_obj_set_style_text_font(ui_settings_screen, font_normal, 0);

    lv_obj_add_event_cb(ui_settings_screen, ui_settings_screen_events_callback, LV_EVENT_ALL, NULL);

    // Settings tab
    {
        auto ui_settings_screen_tab_settings = lv_tabview_add_tab(ui_settings_screen_tab, "Settings");

        // Settings - Brightness panel
        {
            static const lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
            static const lv_coord_t grid_main_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

            auto brightness_panel = lv_obj_create(ui_settings_screen_tab_settings);
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
            ui_settings_screen_tab_settings_brightness_slider = lv_slider_create(brightness_panel);
            lv_obj_set_width(ui_settings_screen_tab_settings_brightness_slider, lv_pct(97));
            lv_slider_set_range(ui_settings_screen_tab_settings_brightness_slider, 1, 255);
            lv_obj_add_event_cb(ui_settings_screen_tab_settings_brightness_slider, ui_settings_screen_tab_settings_brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            lv_obj_refresh_ext_draw_size(ui_settings_screen_tab_settings_brightness_slider);
            lv_obj_set_grid_cell(ui_settings_screen_tab_settings_brightness_slider, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 2, 1);
        }
    }

    // Information tab
    {
        auto ui_settings_screen_tab_information = lv_tabview_add_tab(ui_settings_screen_tab, "Information");

        ui_settings_screen_tab_information_table = lv_table_create(ui_settings_screen_tab_information);
        lv_obj_set_size(ui_settings_screen_tab_information_table, lv_pct(100), LV_SIZE_CONTENT);
    }
}

void ui_init()
{
    lv_disp_t *dispp = lv_disp_get_default();
    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                              false, LV_FONT_DEFAULT);

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    lv_style_init(&style_label_default);
    lv_style_set_text_font(&style_label_default, font_normal);

    lv_disp_set_theme(dispp, theme);
    ui_bootscreen_screen_init();
    ui_main_screen_screen_init();
    ui_settings_screen_screen_init();
    lv_disp_load_scr(ui_bootscreen);
}

void ui_set_aqi_value(uint16_t value)
{
    lv_label_set_text_fmt(ui_aqi_value_label, "%d", value);

    uint32_t color;
    if (value < 50)
    {
        color = 0x35e41d; // green
    }
    else if (value < 100)
    {
        color = 0xc6bd15; // yellow
    }
    else if (value < 150)
    {
        color = 0xcd6f1b; // orange
    }
    else if (value < 200)
    {
        color = 0xd32514; // red
    }
    else if (value < 200)
    {
        color = 0x866846;
    }

    lv_obj_set_style_text_color(ui_aqi_value_label, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
}
