#pragma once

#include "sensor_id.h"

class ui_inter_screen_interface
{
public:
    virtual void show_home_screen() = 0;
    virtual void show_setting_screen() = 0;
    virtual void show_sensor_detail_screen(sensor_id_index index) = 0;
};