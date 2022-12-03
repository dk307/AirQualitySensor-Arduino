#include "hardware.h"

const std::array<sensor_definition_display, 2> hardware::aqi_definition_display{
    sensor_definition_display{0, 50, sensor_level::_1},
    sensor_definition_display{50, 100, sensor_level::_2},
};

const std::array<sensor_definition_display, 1> hardware::voc_definition_display{
    sensor_definition_display{-99, 99, sensor_level::_1}, 
};

const std::array<sensor_definition_display, 1> hardware::co2_definition_display{
    sensor_definition_display{2000, 2000, sensor_level::_1}, 
};

const std::array<sensor_definition_display, 7> hardware::temperature_definition_display{
    sensor_definition_display{-99, 0, sensor_level::_5},
    sensor_definition_display{0, 40, sensor_level::_4},
    sensor_definition_display{40, 65, sensor_level::_2},
    sensor_definition_display{65, 85, sensor_level::_1},
    sensor_definition_display{85, 90, sensor_level::_2},
    sensor_definition_display{90, 110, sensor_level::_4},
    sensor_definition_display{110, 999, sensor_level::_5},
};
