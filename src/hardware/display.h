#pragma once

#include "lgfxdevice.h"

class display
{
public:
    bool pre_begin();
    void begin();
    void loop();

    static display instance;

    void update_boot_message(const std::string& message);
    void set_main_screen();
    void set_aqi_value(uint16_t value);

private:
    display() = default;

    LGFX display_device;

    lv_disp_draw_buf_t draw_buf{};
    lv_disp_drv_t disp_drv{};
    lv_indev_drv_t indev_drv{};
    lv_disp_t *lv_display{};
    lv_color_t *disp_draw_buf{};
    lv_color_t *disp_draw_buf2{};

    static void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    static void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
};