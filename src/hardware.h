#pragma once

#include "sensor.h"

class hardware
{
public:
    bool pre_begin();
    void begin();
    void loop();

    static hardware instance;

    enum class sensor_id_index
    {
        aqi = 0,
        voc,
        co2,
        temperatureF,
    };

    const sensor_definition &get_sensor(sensor_id_index index) const
    {
        return sensors[static_cast<uint8_t>(index)];
    }

private:
    hardware() = default;

    const static std::array<sensor_definition_display, 2> aqi_definition_display;
    const static std::array<sensor_definition_display, 1> voc_definition_display;
    const static std::array<sensor_definition_display, 1> co2_definition_display;
    const static std::array<sensor_definition_display, 7> temperature_definition_display;

    sensor_definition sensors[4]{
        sensor_definition{"AQI", "", aqi_definition_display.data(), aqi_definition_display.size()},
        sensor_definition{"Voc", "", voc_definition_display.data(), voc_definition_display.size()},
        sensor_definition{"CO2", "", co2_definition_display.data(), co2_definition_display.size()},
        sensor_definition{"Temperature", "F", temperature_definition_display.data(), temperature_definition_display.size()},
    };
};
