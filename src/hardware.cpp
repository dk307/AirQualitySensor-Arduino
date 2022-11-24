#include "hardware.h"

#include "hardware/display.h"
#include "hardware/sdcard.h"

hardware hardware::instance;

bool hardware::pre_begin()
{
    if (!display::instance.pre_begin())
    {
        return false;
    }

    if (!sdcard::instance.pre_begin())
    {
        return false;
    }
    return true;
}

void hardware::begin()
{
    display::instance.begin();
    sdcard::instance.begin();
}

void hardware::loop()
{
    display::instance.loop();
    sdcard::instance.loop();
}
