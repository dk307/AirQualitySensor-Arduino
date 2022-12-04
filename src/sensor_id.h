#pragma once

#include <stdint.h>

enum class sensor_id_index
{
    aqi = 0,
    voc,
    co2,
    temperatureF,
    last = temperatureF
};

typedef uint8_t sensor_level;
