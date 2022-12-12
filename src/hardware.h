#pragma once

#include "sensor.h"
#include "task_wrapper.h"
#include "psram_allocator.h"

class hardware
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

private:
    hardware() = default;

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
};
