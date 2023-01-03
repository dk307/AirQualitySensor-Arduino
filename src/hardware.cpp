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
#include <esp_netif_types.h>
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

void hardware::set_screen_brightness(uint8_t value)
{
    log_i("Setting display brightness to %d", value);
    display_instance.set_brightness(value);
}

ui_interface::information_table_type hardware::get_information_table(information_type type)
{
    switch (type)
    {
    case information_type::system:
        return {
            {F("Chip"), to_string(ESP.getChipModel(), "\nRev: ", ESP.getChipRevision(), "\nFlash: ", stringify_size(ESP.getFlashChipSize()))},
            {F("Heap"), to_string(stringify_size(ESP.getFreeHeap()), " free out of ", stringify_size(ESP.getHeapSize()))},
            {F("PsRam"), to_string(stringify_size(ESP.getFreePsram(), 1), " free out of ", stringify_size(ESP.getPsramSize(), 1))},
            {F("Uptime"), get_up_time()},
            {F("SD Card Size:"), to_string(SD.cardSize() / (1024 * 1024), " MB")},
        };

    case information_type::network:
    {
        ui_interface::information_table_type table;
        switch (WiFi.getMode())
        {
        case WIFI_MODE_STA:
        {
            table.push_back({"Mode", "STA Mode"});

            wifi_ap_record_t info;
            const auto result_info = esp_wifi_sta_get_ap_info(&info);
            if (result_info != ESP_OK)
            {
                table.push_back({"Error", to_string("failed to get info with error", result_info)});
            }
            else
            {
                table.push_back({"SSID", reinterpret_cast<char *>(info.ssid)});
                table.push_back({"Hostname", WiFi.getHostname()});
                table.push_back({"Mac Address", WiFi.softAPmacAddress()});
                table.push_back({"RSSI", to_string(info.rssi)});
                table.push_back({"IP Address(wifi)", WiFi.localIP().toString()});
                table.push_back({"Gateway Address", WiFi.gatewayIP().toString()});
                table.push_back({"Subnet", WiFi.subnetMask().toString()});
                table.push_back({"DNS", WiFi.dnsIP().toString()});
            }
        }
        break;
        case WIFI_MODE_AP:
            table.push_back({"Mode", "Access Point Mode"});
            table.push_back({"SSID", WiFi.softAPSSID()});
            break;
        case WIFI_MODE_APSTA:
            table.push_back({"Mode", "AP+STA Mode"});
            break;
        }

        return table;
    }
    }
    return {}; 
}

std::optional<sensor_value::value_type> hardware::get_sensor_value(sensor_id_index index) const
{
    auto &&sensor = get_sensor(index);
    return sensor.get_value();
}

sensor_history::sensor_history_snapshot hardware::get_sensor_detail_info(sensor_id_index index)
{
    return (*sensors_history)[static_cast<size_t>(index)].get_snapshot();
}

bool hardware::is_wifi_connected()
{
    return wifi_manager::instance.is_wifi_connected();
}

String hardware::get_wifi_status()
{
    StreamString stream;
    switch (WiFi.getMode())
    {
    case WIFI_MODE_STA:
    {
        wifi_ap_record_t info;
        const auto result_info = esp_wifi_sta_get_ap_info(&info);
        if (result_info != ESP_OK)
        {
            stream.printf("STA Mode, failed to get info with error:%d", result_info);
        }
        else
        {
            stream.printf("Connected to %s with IP %s", reinterpret_cast<char *>(info.ssid), WiFi.localIP().toString().c_str());
        }
        break;
    }
    case WIFI_MODE_AP:
        stream.printf("Access Point with SSID:%s", WiFi.softAPSSID().c_str());
        break;
    case WIFI_MODE_APSTA:
        stream.print("AP+STA Mode");
        break;
    }
    return stream;
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
                                                            log_i("Hardware task started on Core:%d", xPortGetCoreID());
                                                            do
                                                            {
                                                                display_instance.loop();
                                                                read_sensors();                                                                    
                                                                vTaskDelay(5);
                                                            } while(true); });

    // start on core 0
    sensor_read_task->spawn_arduino_other_core("hardware task");
}

void hardware::read_sensors()
{
    const auto sensor_interval = (60 * 1000 / sensor_history::reads_per_minute);
    const auto now = millis();
    if (now - sensor_last_read >= sensor_interval)
    {
        sensor_last_read = now;
        log_i("Reading sensors");
        int plus = esp_random() % 2 == 1 ? -1 : 1;
        set_sensor_value(sensor_id_index::pm_2_5, (get_sensor_value(sensor_id_index::pm_2_5).value_or(0) + plus * esp_random() % 10) % 250);
        set_sensor_value(sensor_id_index::eCO2, (get_sensor_value(sensor_id_index::eCO2).value_or(0) + plus * esp_random() % 10) % 1999);
        set_sensor_value(sensor_id_index::temperatureF, (get_sensor_value(sensor_id_index::temperatureF).value_or(0) + plus * esp_random() % 3) % 120);
        set_sensor_value(sensor_id_index::humidity, (get_sensor_value(sensor_id_index::humidity).value_or(0) - plus * esp_random() % 3) % 99);
    }
}