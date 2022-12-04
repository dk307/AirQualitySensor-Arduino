#include "hardware.h"

const std::array<sensor_definition_display, 3> hardware::aqi_definition_display{
    sensor_definition_display{0, 50, 0},
    sensor_definition_display{50, 100, 1},
    sensor_definition_display{100, 200, 2},
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
