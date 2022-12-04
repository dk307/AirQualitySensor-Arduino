#pragma once

#include "sensor.h"
#include "task_wrapper.h"

class hardware
{
public:
    bool pre_begin();
    void begin();
    void loop();

    static hardware instance;

    sensor_definition &get_sensor(sensor_id_index index)
    {
        return sensors[static_cast<uint8_t>(index)];
    }

private:
    hardware() = default;
    
 
    const static std::array<sensor_definition_display, 3> aqi_definition_display;
    const static std::array<sensor_definition_display, 1> voc_definition_display;
    const static std::array<sensor_definition_display, 1> co2_definition_display;
    const static std::array<sensor_definition_display, 1> temperature_definition_display;

    // same index as sensor_id_index
    std::array<sensor_definition, 4> sensors{
        sensor_definition{"AQI", "", aqi_definition_display.data(), aqi_definition_display.size()},
        sensor_definition{"Voc", "", voc_definition_display.data(), voc_definition_display.size()},
        sensor_definition{"CO2", "", co2_definition_display.data(), co2_definition_display.size()},
        sensor_definition{"Temperature", "F", temperature_definition_display.data(), temperature_definition_display.size()},
    };

    std::unique_ptr<task_wrapper> sensor_read_task;
};
