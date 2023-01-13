#pragma once

#include <stdint.h>

enum class sensor_id_index : uint8_t
{
    pm_2_5 = 0,
    voc,
    temperatureF,
    humidity,
    eCO2,
    pm_1,
    pm_4,
    pm_10,
    typical_particle_size,
    last = typical_particle_size,
};

constexpr size_t total_sensors = static_cast<size_t>(sensor_id_index::last ) + 1;

typedef uint8_t sensor_level;
