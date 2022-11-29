#include "display.h"
#include "ui/ui2.h"
#include "config_manager.h"

display display::instance;

/* Display flushing */
void display::display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    auto &display_device = display::instance.display_device;
    if (display_device.getStartCount() == 0)
    {
        display_device.endWrite();
    }

    display_device.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1,
                                (lgfx::swap565_t *)&color_p->full);

    lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

/*Read the touchpad*/
void display::touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    auto &display_device = display::instance.display_device;
    uint16_t touchX, touchY;
    const bool touched = display_device.getTouch(&touchX, &touchY);

    if (!touched)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
        log_v("Touch at point %d:%d", touchX, touchY);
    }
}

bool display::pre_begin()
{
    std::lock_guard<std::mutex> lock(lgvl_mutex);
    lv_init();

    if (!display_device.init())
    {
        log_e("Failed to init display");
    }
    display_device.setRotation(1);

    display_device.initDMA();
    display_device.startWrite();

    const auto screenWidth = display_device.width();
    const auto screenHeight = display_device.height();

    log_i("Display initialized width:%d height:%d", screenWidth, screenHeight);

    log_d("LV initialized");
    const int buffer_size = 40;

    const auto display_buffer_size = screenWidth * buffer_size * sizeof(lv_color_t);
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(display_buffer_size, MALLOC_CAP_DMA);
    disp_draw_buf2 = (lv_color_t *)heap_caps_malloc(display_buffer_size, MALLOC_CAP_DMA);

    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, disp_draw_buf2, screenWidth * buffer_size);

    log_d("LVGL display buffer initialized");

    /*** LVGL : Setup & Initialize the display device driver ***/
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = display_device.width();
    disp_drv.ver_res = display_device.height();
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

    ui_init();

    lv_timer_handler();

    log_i("Done");
    return true;
}

void display::begin()
{
    auto brightness = config::instance.data.get_manual_screen_brightness();
    display_device.setBrightness(brightness.value_or(128));
}

void display::loop()
{
    std::lock_guard<std::mutex> lock(lgvl_mutex);
    lv_timer_handler();
}

void display::update_boot_message(const std::string &message)
{
    std::lock_guard<std::mutex> lock(lgvl_mutex);
    lv_label_set_text(ui_boot_message, message.c_str());
}

void display::set_main_screen()
{
    std::lock_guard<std::mutex> lock(lgvl_mutex);
    log_i("Switching to main screen");
    lv_disp_load_scr(ui_main_screen);
}

void display::set_aqi_value(uint16_t value)
{
    std::lock_guard<std::mutex> lock(lgvl_mutex);
    ui_set_aqi_value(value);
}

uint8_t display::get_brightness()
{
    return display_device.getBrightness();
}

void display::set_brightness(uint8_t value)
{
    display_device.setBrightness(value);
}