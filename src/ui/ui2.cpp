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
    log_printf("%s", dsc);
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
    common_fonts.font_montserrat_regular_numbers_48 = lv_font_load("S:display/font/montserrat/ui_font_m48regularnumbers.bin");
    log_d("2");
    common_fonts.font_montserrat_regular_numbers_40 = lv_font_load("S:display/font/montserrat/ui_font_m40regularnumbers.bin");
    log_d("3");
    common_fonts.font_montserrat_light_numbers_112 = lv_font_load("S:display/font/montserrat/ui_font_m112lightnumbers.bin");
    log_d("4");
    common_fonts.font_montserrat_medium_48 = lv_font_load("S:display/font/montserrat/ui_font_m48medium.bin");
    log_d("5");
    common_fonts.font_montserrat_medium_14 = lv_font_load("S:display/font/montserrat/ui_font_m14medium.bin");
    log_d("6");
    common_fonts.font_montserrat_medium_units_18 = lv_font_load("S:display/font/montserrat/ui_font_m18unitsmedium.bin");

    log_d("Loaded From SD Card");
}

void ui::init()
{
    lv_log_register_print_cb(&lv_logger);
    lv_disp_t *dispp = lv_disp_get_default();

    lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE_GREY), lv_palette_main(LV_PALETTE_BROWN),
                                              false, LV_FONT_DEFAULT);

    lv_disp_set_theme(dispp, theme);

    boot_screen.init();
    boot_screen.show_screen();

    inline_loop(100);

    log_i("Loaded boot screen");

    boot_screen.set_boot_message("Loading from SD Card");
    inline_loop(50);

    load_from_sd_card(); // might take some time

    main_screen.init();
    sensor_detail_screen.init();
    settings_screen.init();
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

void ui::set_main_screen()
{
    main_screen.show_screen();
}

void ui::update_configuration()
{
    settings_screen.update_configuration();
}
