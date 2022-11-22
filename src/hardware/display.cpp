#include "display.h"

#include <demos\widgets\lv_demo_widgets.h>

display display::instance;

/* Display flushing */
void display::display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    auto &displayDevice = display::instance.displayDevice;
    if (displayDevice.getStartCount() == 0)
    {
        displayDevice.endWrite();
    }

    displayDevice.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1,
                               (lgfx::swap565_t *)&color_p->full);

    lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/*Read the touchpad*/
void display::touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    auto &displayDevice = display::instance.displayDevice;
    uint16_t touchX, touchY;
    const bool touched = displayDevice.getTouch(&touchX, &touchY);

    if (!touched)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
        log_d("Touch at point %d:%d", touchX, touchY);
    }
}
 
bool display::pre_begin()
{
    lv_init();

    if (!displayDevice.init())
    {
        log_e("Failed to init display");
    }
    displayDevice.setRotation(1);

    displayDevice.initDMA();
    displayDevice.startWrite();

    const auto screenWidth = displayDevice.width();
    const auto screenHeight = displayDevice.height();

    log_i("Display initialized width:%d  height:%d", screenWidth, screenHeight);

    log_d("LV initialized");
    const int BufferSize = 40;

    const auto displayBufferSize = screenWidth * BufferSize * sizeof(lv_color_t);
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(displayBufferSize, MALLOC_CAP_DMA);
    disp_draw_buf2 = (lv_color_t *)heap_caps_malloc(displayBufferSize, MALLOC_CAP_DMA);

    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, disp_draw_buf2, screenWidth * BufferSize);

    log_d("LVGL display buffer initialized");

    /*** LVGL : Setup & Initialize the display device driver ***/
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = displayDevice.width();
    disp_drv.ver_res = displayDevice.height();
    disp_drv.flush_cb = display_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_display = lv_disp_drv_register(&disp_drv);

    log_d("LVGL display initialized");

    //*** LVGL : Setup & Initialize the input device driver ***F
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

    log_d("LVGL input device driver initialized");

    // ui_init();
    lv_demo_widgets();

    log_i("Done");
    return true;
}

void display::begin()
{
}

void display::loop()
{
    lv_timer_handler();
}
