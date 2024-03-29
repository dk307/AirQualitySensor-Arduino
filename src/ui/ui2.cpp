#include "ui2.h"
#include "ui_interface.h"
#include "config_manager.h"
#include "sensor.h"
#include "ntp_time.h"
#include <task_wrapper.h>

#include <tuple>
#include <memory>

void lv_logger(const char *dsc)
{
    ESP_LOGI(UI_TAG, "%s", dsc);
}

void ui::load_from_sd_card()
{
    if (lv_fs_is_ready('S'))
    {
        ESP_LOGI(UI_TAG, "lv fs is ready. Loading from SD Card");
    }
    else
    {
        log_e("lv fs not ready");
    }

    common_fonts.font_montserrat_regular_numbers_40 = lv_font_load("S:display/font/montserrat/ui_font_m40regularnumbers.bin");
    common_fonts.font_big_panel = lv_font_load("S:display/font/big_panel_top.bin");
    common_fonts.font_montserrat_medium_48 = lv_font_load("S:display/font/montserrat/ui_font_m48medium.bin");
    common_fonts.font_montserrat_medium_14 = lv_font_load("S:display/font/montserrat/ui_font_m14medium.bin");
    common_fonts.font_montserrat_medium_units_18 = lv_font_load("S:display/font/montserrat/ui_font_m18unitsmedium.bin");
    common_fonts.font_temp_hum = lv_font_load("S:display/font/temp_hum.bin");

    ESP_LOGI(UI_TAG,"Loaded From SD Card");
}

void ui::no_wifi_img_animation_cb(void *var, int32_t v)
{
    auto pThis = (ui *)var;
    const auto op = v > 256 ? 512 - v : v;
    lv_style_set_img_opa(&pThis->no_wifi_image_style, op);
    lv_obj_refresh_style(pThis->no_wifi_image, LV_PART_ANY, LV_STYLE_PROP_ANY);
}

void ui::init()
{
    lv_log_register_print_cb(&lv_logger);
    lv_disp_t *dispp = lv_disp_get_default();

    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_LIME),
                                              true, LV_FONT_DEFAULT);

    lv_disp_set_theme(dispp, theme);

    boot_screen.init();
    boot_screen.show_screen();

    inline_loop(100);

    ESP_LOGI(UI_TAG, "Loaded boot screen");

    boot_screen.set_boot_message("Loading from SD Card");
    inline_loop(50);

    load_from_sd_card(); // might take some time

    init_top_message();
    init_no_wifi_image();

    main_screen.init();
    launcher_screen.init();
    sensor_detail_screen.init();
    settings_screen.init();
    hardware_info_screen.init();
}

void ui::init_no_wifi_image()
{
    no_wifi_image = lv_img_create(lv_layer_sys());
    lv_img_set_src(no_wifi_image, "S:display/image/nowifi.png");
    lv_obj_align(no_wifi_image, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_add_flag(no_wifi_image, LV_OBJ_FLAG_HIDDEN);

    lv_style_init(&no_wifi_image_style);
    lv_style_set_img_opa(&no_wifi_image_style, 100);
    lv_obj_add_style(no_wifi_image, &no_wifi_image_style, 0);

    lv_anim_t no_wifi_image_animation;
    lv_anim_init(&no_wifi_image_animation);
    lv_anim_set_var(&no_wifi_image_animation, this);
    lv_anim_set_values(&no_wifi_image_animation, 0, 512);
    lv_anim_set_time(&no_wifi_image_animation, 2000);
    lv_anim_set_exec_cb(&no_wifi_image_animation, no_wifi_img_animation_cb);
    lv_anim_set_repeat_count(&no_wifi_image_animation, LV_ANIM_REPEAT_INFINITE);

    no_wifi_image_animation_timeline = lv_anim_timeline_create();
    lv_anim_timeline_add(no_wifi_image_animation_timeline, 0, &no_wifi_image_animation);
}

void ui::top_message_timer_cb(lv_timer_t *e)
{
    auto p_this = reinterpret_cast<ui *>(e->user_data);
    lv_obj_add_flag(p_this->top_message_panel, LV_OBJ_FLAG_HIDDEN);
    lv_timer_pause(p_this->top_message_timer);
}

void ui::init_top_message()
{
    top_message_panel = lv_obj_create(lv_layer_sys());
    lv_obj_set_size(top_message_panel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(top_message_panel, LV_ALIGN_TOP_MID, 0, 15);
    lv_obj_set_style_border_width(top_message_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(top_message_panel, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(top_message_panel, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(top_message_panel, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(top_message_panel, lv_color_hex(0xF5F5F5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(top_message_panel, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_EVENT_BUBBLE | LV_OBJ_FLAG_GESTURE_BUBBLE);

    top_message_label = lv_label_create(top_message_panel);
    lv_obj_set_size(top_message_panel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(top_message_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_long_mode(top_message_label, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(top_message_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(top_message_label, &lv_font_montserrat_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(top_message_label, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);

    top_message_timer = lv_timer_create(top_message_timer_cb, top_message_timer_period, this);
    lv_timer_pause(top_message_timer);
}

void ui::inline_loop(uint64_t maxWait)
{
    const auto now = millis();
    while (millis() - now < maxWait)
    {
        lv_timer_handler();
        delay(3);
    }
}

void ui::set_sensor_value(sensor_id_index index, const std::optional<sensor_value::value_type> &value)
{
    if (main_screen.is_active())
    {
        main_screen.set_sensor_value(index, value);
    }
    if (sensor_detail_screen.is_active())
    {
        sensor_detail_screen.set_sensor_value(index, value);
    }
}

void ui::update_boot_message(const String &message)
{
    boot_screen.set_boot_message(message);
    inline_loop(50);
}

void ui::show_top_level_message(const String &message, uint32_t period)
{
    ESP_LOGI(UI_TAG, "Showing top level message:%s", message.c_str());
    lv_label_set_text(top_message_label, message.c_str());
    lv_obj_clear_flag(top_message_panel, LV_OBJ_FLAG_HIDDEN);
    lv_timer_reset(top_message_timer);
    lv_timer_resume(top_message_timer);
}

void ui::set_main_screen()
{
    main_screen.show_screen();
    wifi_changed();
}

void ui::wifi_changed()
{
    if (!boot_screen.is_active())
    {
        if (ui_interface_instance.is_wifi_connected())
        {
            ESP_LOGI(UI_TAG, "Hiding No wifi icon");
            lv_obj_add_flag(no_wifi_image, LV_OBJ_FLAG_HIDDEN);
            lv_anim_timeline_stop(no_wifi_image_animation_timeline);
        }
        else
        {
            ESP_LOGI(UI_TAG, "Showing No wifi icon");
            lv_obj_clear_flag(no_wifi_image, LV_OBJ_FLAG_HIDDEN);
            lv_anim_timeline_start(no_wifi_image_animation_timeline);
        }

        show_top_level_message(ui_interface_instance.get_wifi_status(), 5000);
    }
}
