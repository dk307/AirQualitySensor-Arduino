#include "hardware.h"

#include "hardware/display.h"

hardware hardware::instance;

bool hardware::pre_begin()
{
    if (!display::instance.pre_begin())
    {
        return false;
    }
    return true;
}

void hardware::begin()
{
}

void hardware::loop()
{
    display::instance.loop();
}
