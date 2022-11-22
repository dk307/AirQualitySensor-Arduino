#pragma once

#include "lgfxdevice.h"

class display
{
public:
    bool pre_begin();
    void begin();
    void loop();

    static display instance;

private:
    display() = default;

    LGFX displayDevice;

    lv_disp_draw_buf_t draw_buf{};
    lv_disp_drv_t disp_drv{};
    lv_indev_drv_t indev_drv{};
    lv_disp_t *lv_display{};
    lv_color_t *disp_draw_buf{};
    lv_color_t *disp_draw_buf2{};

    static void display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    static void touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
};