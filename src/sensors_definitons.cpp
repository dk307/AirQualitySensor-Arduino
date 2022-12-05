#include "hardware.h"

#include <limits>

const std::array<sensor_definition_display, 6> hardware::aqi_definition_display{
    sensor_definition_display{std::numeric_limits<uint32_t>::min(), 50, 0},
    sensor_definition_display{50, 100, 1},
    sensor_definition_display{100, 150, 2},
    sensor_definition_display{151, 200, 3},
    sensor_definition_display{201, 300, 4},
    sensor_definition_display{300, std::numeric_limits<uint32_t>::max(), 5},
};

const std::array<sensor_definition_display, 1> hardware::voc_definition_display{
    sensor_definition_display{-99, 99, 0},
};

const std::array<sensor_definition_display, 1> hardware::co2_definition_display{
    sensor_definition_display{2000, 2000, 0},
};

const std::array<sensor_definition_display, 1> hardware::temperature_definition_display{
    sensor_definition_display{-99, 99, 0},
};
