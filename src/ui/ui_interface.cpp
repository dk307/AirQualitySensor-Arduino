#include "ui_interface.h"

#include <Arduino.h>
#include <Wifi.h>
#include <StreamString.h>

#include <memory>
#include <sstream>
#include <iomanip>

ui_interface ui_interface::instance;

template <class T>
inline Print &operator<<(Print &obj, T &&arg)
{
    obj.print(std::forward<T>(arg));
    return obj;
}

template <class... Args>
String to_string(Args &&...args)
{
    StreamString stream;
    (stream << ... << std::forward<Args>(args));
    return stream;
}

String stringify_size(uint64_t bytes)
{
    constexpr char suffix[3][3] = {"B", "KB", "MB"};
    constexpr char length = sizeof(suffix) / sizeof(suffix[0]);

    uint16_t i = 0;
    double dblBytes = bytes;

    if (bytes > 1024)
    {
        for (i = 0; (bytes / 1024) > 0 && i < length - 1; i++, bytes /= 1024)
        {
            dblBytes = bytes / 1024.0;
        }
    }

    return to_string(static_cast<uint64_t>(std::round(dblBytes)), ' ', suffix[i]);
}

String ui_interface::get_up_time()
{
    const auto now = millis() / 1000;
    const auto hour = now / 3600;
    const auto mins = (now % 3600) / 60;
    const auto sec = (now % 3600) % 60;

    StreamString upTime;
    upTime.reserve(30U);
    upTime.printf_P(PSTR("%02d hours %02d mins %02d secs"), hour, mins, sec);
    return upTime;
}

ui_interface::information_table_type ui_interface::get_information_table()
{
    return {
        {F("Chip"), to_string(ESP.getChipModel(), " Flash: ", stringify_size(ESP.getFlashChipSize()))},
        {"Heap", to_string(stringify_size(ESP.getFreeHeap()), " free out of ", stringify_size(ESP.getHeapSize()))},
        {"PsRam", to_string(stringify_size(ESP.getFreePsram()), " free out of ", stringify_size(ESP.getPsramSize()))},
        {"Uptime", get_up_time()},
        {"Mac Address", WiFi.softAPmacAddress()},
    };
}