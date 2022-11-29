#include "ui_interface.h"
#include "wifi_manager.h"
#include "config_manager.h"
#include "hardware/display.h"

#include <Arduino.h>
#include <Wifi.h>
#include <esp_wifi.h>
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

String stringify_size(uint64_t bytes, int max_unit = 128)
{
    constexpr char suffix[3][3] = {"B", "KB", "MB"};
    constexpr char length = sizeof(suffix) / sizeof(suffix[0]);

    uint16_t i = 0;
    double dblBytes = bytes;

    if (bytes > 1024)
    {
        for (i = 0; (bytes / 1024) > 0 && i < length - 1 && (max_unit > 0); i++, bytes /= 1024)
        {
            dblBytes = bytes / 1024.0;
            max_unit--;
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

String ui_interface::network_status()
{
    StreamString stream;
    switch (WiFi.getMode())
    {
    case WIFI_MODE_STA:
    {
        stream.print("STA Mode");
        wifi_ap_record_t info;

        const auto result_info = esp_wifi_sta_get_ap_info(&info);
        if (result_info != ESP_OK)
        {
            stream.printf(", failed to get info with :d", result_info);
        }
        else
        {
            stream.printf("\nSsid:%s\nIP:%s\nRSSI:%d db", reinterpret_cast<char *>(info.ssid), WiFi.localIP().toString().c_str(), info.rssi);
        }
    }
    break;
    case WIFI_MODE_AP:
        stream.print("AP Mode");
        break;
    case WIFI_MODE_APSTA:
        stream.print("AP+STA Mode");
        break;
    }
    return stream;
}

uint8_t ui_interface::get_manual_screen_brightness()
{
    return display::instance.get_brightness();
}

void ui_interface::set_manual_screen_brightness(uint8_t value)
{
    log_i("Setting display brightness to %d", value);
    display::instance.set_brightness(value);

    config::instance.data.set_manual_screen_brightness(value);
    config::instance.save();
}

ui_interface::information_table_type ui_interface::get_information_table()
{
    return {
        {F("Chip"), to_string(ESP.getChipModel(), "\nRev: ", ESP.getChipRevision(), "\nFlash: ", stringify_size(ESP.getFlashChipSize()))},
        {F("Heap"), to_string(stringify_size(ESP.getFreeHeap()), " free out of ", stringify_size(ESP.getHeapSize()))},
        {F("PsRam"), to_string(stringify_size(ESP.getFreePsram(), 1), " free out of ", stringify_size(ESP.getPsramSize(), 1))},
        {F("Uptime"), get_up_time()},
        {F("Mac Address"), WiFi.softAPmacAddress()},
        {F("Captive portal"), wifi_manager::instance.is_captive_portal() ? "Yes" : "No"},
        {F("Network"), network_status()},
    };
}