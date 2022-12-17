#pragma once

#include <WString.h>
#include <vector>
#include <string>
#include <psram_allocator.h>
#include "sensor_id.h"
#include "sensor.h"

class ui_interface
{
public:
    typedef std::vector<std::pair<String, String>, psram::allocator<std::pair<String, String>>> information_table_type;
    virtual information_table_type get_information_table() = 0;

    virtual uint8_t get_manual_screen_brightness() = 0;
    virtual void set_manual_screen_brightness(uint8_t value) = 0;
    virtual std::optional<sensor_value::value_type> get_sensor_value(sensor_id_index index) const = 0;
    virtual sensor_history::sensor_history_snapshot get_sensor_detail_info(sensor_id_index index) = 0;
};