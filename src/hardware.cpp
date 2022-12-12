#include "hardware.h"

#include "hardware/display.h"
#include "hardware/sdcard.h"

hardware hardware::instance;

bool hardware::pre_begin()
{
    sensors_history = psram::make_unique<std::array<sensor_history, total_sensors>>();
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

    sensor_read_task = std::make_unique<task_wrapper>([this]
                                                      {
                                                                      do
                                                                      {
                                                                          log_d("Core:%d", xPortGetCoreID());                                                                       
                                                                          set_sensor_value(sensor_id_index::pm_2_5, esp_random() % 999);
                                                                          sensors[1].set_value(esp_random() % 500);
                                                                          sensors[2].set_value(esp_random() % 99);
                                                                          sensors[3].set_value(esp_random() % 99);                                                
                                                                          sensors[4].set_value(esp_random() % 2000);
                                                                          sensors[5].set_value(esp_random() % 999);
                                                                          sensors[6].set_value(esp_random() % 999);
                                                                          sensors[7].set_value(esp_random() % 999);
                                                                          vTaskDelay(5000);
                                                                      } while(true); });

    sensor_read_task->spawn_pinned("sensor read task", 8192, 1, 0);                                                                
}

void hardware::loop()
{
    display::instance.loop();
}
