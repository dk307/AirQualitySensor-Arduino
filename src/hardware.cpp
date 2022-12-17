#include "hardware.h"

#include "hardware/display.h"
#include "hardware/sdcard.h"
#include "wifi_manager.h"
#include "config_manager.h"
#include "hardware/display.h"
#include "hardware.h"

#include <Arduino.h>
#include <Wifi.h>
#include <esp_wifi.h>
#include <StreamString.h>
#include <SD.h>

#include <memory>
#include <sstream>
#include <iomanip>

hardware hardware::instance;

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

String hardware::get_up_time()
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

String hardware::network_status()
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
            stream.printf(", failed to get info with error:%d", result_info);
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

uint8_t hardware::get_manual_screen_brightness()
{
    return display_instance.get_brightness();
}

void hardware::set_manual_screen_brightness(uint8_t value)
{
    log_i("Setting display brightness to %d", value);
    display_instance.set_brightness(value);

    config::instance.data.set_manual_screen_brightness(value);
    config::instance.save();
}

ui_interface::information_table_type hardware::get_information_table()
{
    return {
        {F("Chip"), to_string(ESP.getChipModel(), "\nRev: ", ESP.getChipRevision(), "\nFlash: ", stringify_size(ESP.getFlashChipSize()))},
        {F("Heap"), to_string(stringify_size(ESP.getFreeHeap()), " free out of ", stringify_size(ESP.getHeapSize()))},
        {F("PsRam"), to_string(stringify_size(ESP.getFreePsram(), 1), " free out of ", stringify_size(ESP.getPsramSize(), 1))},
        {F("Uptime"), get_up_time()},
        {F("Mac Address"), WiFi.softAPmacAddress()},
        {F("Captive portal"), wifi_manager::instance.is_captive_portal() ? "Yes" : "No"},
        {F("Network"), network_status()},
        {F("SD Card Size:"), to_string(SD.cardSize() / (1024 * 1024), " MB")},
    };
}

std::optional<sensor_value::value_type> hardware::get_sensor_value(sensor_id_index index) const
{
    auto && sensor = get_sensor(index);
    return sensor.get_value();
}

sensor_history::sensor_history_snapshot hardware::get_sensor_detail_info(sensor_id_index index)
{
   return (*sensors_history)[static_cast<size_t>(index)].get_snapshot();
}

bool hardware::pre_begin()
{
    sensors_history = psram::make_unique<std::array<sensor_history, total_sensors>>();
    if (!sdcard::instance.pre_begin())
    {
        return false;
    }

    if (!display_instance.pre_begin())
    {
        return false;
    }
    return true;
}

void hardware::begin()
{
    display_instance.begin();
    sdcard::instance.begin();

    sensor_read_task = std::make_unique<task_wrapper>([this]
                                                      {
                                                                      do
                                                                      {
                                                                          log_d("Core:%d", xPortGetCoreID());                                                                       
                                                                          set_sensor_value(sensor_id_index::pm_2_5, esp_random() % 250);
                                                                          set_sensor_value(sensor_id_index::temperatureF, esp_random() % 99);
                                                                          set_sensor_value(sensor_id_index::humidity, esp_random() % 99);                                                                      
                                                                          vTaskDelay(5000);
                                                                      } while(true); });

    sensor_read_task->spawn_pinned("sensor read task", 8192, 1, 0);
}

void hardware::loop()
{
    display_instance.loop();
}
