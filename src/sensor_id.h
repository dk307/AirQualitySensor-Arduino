#pragma once

#include <stdint.h>

enum class sensor_id_index : std::size_t
{
    pm_2_5 = 0,
    voc,
    co2,
    temperatureF,
    humidity,
    eCO2,
    pm_1,
    pm_4,
    pm_10,
    last = pm_10,
};

constexpr size_t total_sensors = static_cast<size_t>(sensor_id_index::last);

typedef uint8_t sensor_level;
