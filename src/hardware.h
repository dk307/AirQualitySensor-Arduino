#pragma once

#include "sensor.h"
#include "task_wrapper.h"
#include "psram_allocator.h"
#include "hardware/display.h"
#include "ui/ui_interface.h"

class hardware final : ui_interface
{
public:
    bool pre_begin();
    void begin();
    void loop();

    static hardware instance;

    const sensor_value &get_sensor(sensor_id_index index)
    {
        return sensors[static_cast<uint8_t>(index)];
    }

    const sensor_history &get_sensor_history(sensor_id_index index)
    {
        return (*sensors_history)[static_cast<uint8_t>(index)];
    }

    void update_boot_message(const String &message)
    {
        display_instance.update_boot_message(message);
    }
    void set_main_screen()
    {
        display_instance.set_main_screen();
    }

    // ui_interface
    information_table_type get_information_table() override;
    uint8_t get_manual_screen_brightness() override;
    void set_manual_screen_brightness(uint8_t value) override;
    sensor_value::value_type get_sensor_value(sensor_id_index index) override;
    sensor_history::sensor_history_snapshot get_sensor_detail_info(sensor_id_index index) override;

private:
    hardware() = default;

    display display_instance{*this};

    // same index as sensor_id_index
    std::array<sensor_value, total_sensors> sensors;
    std::unique_ptr<std::array<sensor_history, total_sensors>, psram::deleter> sensors_history;

    std::unique_ptr<task_wrapper> sensor_read_task;

    template <class T>
    void set_sensor_value(sensor_id_index index, T value)
    {
        const auto new_value = sensors[static_cast<size_t>(index)].set_value(value);
        (*sensors_history)[static_cast<size_t>(index)].add_value(new_value);
    }

    static String get_up_time();
    static String network_status();
};
