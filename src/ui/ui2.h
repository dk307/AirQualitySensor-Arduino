#pragma once

#include <lvgl.h>
#include <WString.h>

#include "sensor_id.h"
#include "sensor.h"
#include "ui_interface.h"
#include <task_wrapper.h>

#include "ui_boot_screen.h"
#include "ui_main_screen.h"
#include "ui_sensor_detail_screen.h"
#include "ui_settings_screen.h"
#include "ui_inter_screen_interface.h"

class ui : public ui_inter_screen_interface
{
public:
    ui(ui_interface &ui_interface_) : ui_interface_instance(ui_interface_)
    {
    }
    void init();
    void update_boot_message(const String &message);
    void set_sensor_value(sensor_id_index id, const std::optional<sensor_value::value_type> &value);
    void update_configuration();
    void set_main_screen();
    void wifi_changed();

    // ui_inter_screen_interface
    void show_home_screen() override
    {
        main_screen.show_screen();
    }

    void show_setting_screen() override
    {
        settings_screen.show_screen();
    }
    void show_sensor_detail_screen(sensor_id_index index) override
    {
        sensor_detail_screen.show_screen(index);
    }

private:
    ui_interface &ui_interface_instance;

    // top sys layer
    lv_obj_t *no_wifi_image;
    lv_style_t no_wifi_image_style;
    lv_anim_timeline_t *no_wifi_image_animation_timeline;

    ui_common_fonts common_fonts{};

    ui_boot_screen boot_screen{ui_interface_instance, *this, &common_fonts};
    ui_main_screen main_screen{ui_interface_instance, *this, &common_fonts};
    ui_sensor_detail_screen sensor_detail_screen{ui_interface_instance, *this, &common_fonts};
    ui_settings_screen settings_screen{ui_interface_instance, *this, &common_fonts};

    void inline_loop(uint64_t maxWait);
    void load_from_sd_card();
    void init_no_wifi_image();
    static void no_wifi_img_animation_cb(void *var, int32_t v);
};
