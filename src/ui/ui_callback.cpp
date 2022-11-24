#include "ui_callback.h"

#include <Arduino.h>
#include <Wifi.h>

#include <memory>
#include <sstream>
#include <iomanip>

ui_callback ui_callback::instance;

template <class... Args>
std::string to_string(Args &&...args)
{
    std::ostringstream stream;
    stream.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    (stream << ... << std::forward<Args>(args));
    return stream.str();
}

std::string stringify_size(uint64_t bytes)
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

std::vector<std::pair<std::string, std::string>> ui_callback::get_information_table()
{
    return {
        {"Chip", to_string(ESP.getChipModel(), " Flash: ", stringify_size(ESP.getFlashChipSize()))},
        {"Heap", to_string(stringify_size(ESP.getFreeHeap()), " free out of ", stringify_size(ESP.getHeapSize()))},
        {"PSRam", to_string(stringify_size(ESP.getFreePsram()), " free out of ", stringify_size(ESP.getPsramSize()))},
    };
}