#include "hardware.h"

#include "hardware/display.h"
#include "hardware/sdcard.h"

hardware hardware::instance;

bool hardware::pre_begin()
{
    if (!sdcard::instance.pre_begin())
    {
        return false;
    }
    if (!display::instance.pre_begin())
    {
        return false;
    }
    return true;
}

void hardware::begin()
{
    display::instance.begin();
    sdcard::instance.begin();

    sensor_read_task = std::make_unique<task_wrapper>([]
                                                      {
                                                                      do
                                                                      {
                                                                          log_d("Core:%d", xPortGetCoreID());
                                                                          // ui_load_information();
                                                                          vTaskDelay(1000);
                                                                      } while(true); });

    sensor_read_task->spawn_pinned("sensor read task", 8192, 1, 0);                                                                
}

void hardware::loop()
{
    display::instance.loop();
    sdcard::instance.loop();
}
