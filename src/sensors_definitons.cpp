#include "sensor.h"

#include <limits>

const std::array<sensor_definition_display, 6> pm_2_5_definition_display{
    sensor_definition_display{std::numeric_limits<uint32_t>::min(), 12, 0},
    sensor_definition_display{12, 35.4, 1},
    sensor_definition_display{35.4, 55.4, 2},
    sensor_definition_display{55.4, 150.4, 3},
    sensor_definition_display{150.4, 250.4, 4},
    sensor_definition_display{250.4, std::numeric_limits<uint32_t>::max(), 5},
};

const std::array<sensor_definition_display, 6> pm_10_definition_display{
    sensor_definition_display{std::numeric_limits<uint32_t>::min(), 54, 0},
    sensor_definition_display{55, 154, 1},
    sensor_definition_display{154, 254, 2},
    sensor_definition_display{254, 354, 3},
    sensor_definition_display{354, 424, 4},
    sensor_definition_display{424, std::numeric_limits<uint32_t>::max(), 5},
};

const std::array<sensor_definition_display, 1> voc_definition_display{
    sensor_definition_display{-99, 99, 0},
};

const std::array<sensor_definition_display, 1> co2_definition_display{
    sensor_definition_display{2000, 2000, 0},
};

const std::array<sensor_definition_display, 1> temperature_definition_display{
    sensor_definition_display{-99, 99, 0},
};

const std::array<sensor_definition_display, 1> humidity_definition_display{
    sensor_definition_display{-99, 99, 0},
};

const std::array<sensor_definition, total_sensors> sensor_definitions{
    sensor_definition{"PM 2.5", "µg/m3", pm_2_5_definition_display.data(), pm_2_5_definition_display.size()},
    sensor_definition{"VOC", "", voc_definition_display.data(), voc_definition_display.size()},
    sensor_definition{"Temperature", "°F", temperature_definition_display.data(), temperature_definition_display.size()},
    sensor_definition{"Humidity", "⁒", humidity_definition_display.data(), humidity_definition_display.size()},
    sensor_definition{"eCO2", "ppm", co2_definition_display.data(), co2_definition_display.size()},
    sensor_definition{"PM 1", "µg/m3", temperature_definition_display.data(), temperature_definition_display.size()},
    sensor_definition{"PM 4", "µg/m3", temperature_definition_display.data(), temperature_definition_display.size()},
    sensor_definition{"PM 10", "µg/m3", pm_10_definition_display.data(), pm_10_definition_display.size()},
};