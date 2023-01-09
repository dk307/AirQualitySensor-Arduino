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
#include <Wire.h>

#include <memory>
#include <sstream>
#include <iomanip>

hardware hardware::instance;

static const char *timezone_strings[5]{
    "USA Eastern",
    "USA Central",
    "USA Mountain time",
    "USA Arizona",
    "USA Pacific",
};

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
            {"Firmware Version", to_string(VERSION)},
            {"SDK Version", to_string(ESP.getSdkVersion())},
            {"Chip", to_string(ESP.getChipModel(), "\nRev: ", ESP.getChipRevision(), "\nFlash: ", stringify_size(ESP.getFlashChipSize()))},
            {"Heap", to_string(stringify_size(ESP.getFreeHeap()), " free out of ", stringify_size(ESP.getHeapSize()))},
            {"PsRam", to_string(stringify_size(ESP.getFreePsram(), 1), " free out of ", stringify_size(ESP.getPsramSize(), 1))},
            {"Uptime", get_up_time()},
            {"SD Card Size:", to_string(SD.cardSize() / (1024 * 1024), " MB")},
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
                table.push_back({"IP address(wifi)", WiFi.localIP().toString()});
                table.push_back({"Mac address", WiFi.softAPmacAddress()});
                table.push_back({"RSSI (db)", to_string(info.rssi)});
                table.push_back({"Gateway address", WiFi.gatewayIP().toString()});
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
            table.push_back({"AP SSID", WiFi.softAPSSID()});
            table.push_back({"STA SSID", WiFi.SSID()});
            break;
        }

        auto local_time_now = ntp_time::instance.get_local_time();
        if (local_time_now.has_value())
        {
            char value[64];
            tm tm;
            gmtime_r(&local_time_now.value(), &tm);
            table.push_back({"Local time", asctime_r(&tm, value)});
        }
        else
        {
            table.push_back({"Local time", "Not Synced"});
        }

        return table;
    }
    case information_type::config:
        return {
            {"Hostname", to_string(config::instance.instance.data.get_host_name())},
            {"NTP server", to_string(config::instance.instance.data.get_ntp_server())},
            {"NTP server refresh interval", to_string(config::instance.instance.data.get_ntp_server_refresh_interval())},
            {"Time zone", timezone_strings[static_cast<size_t>(config::instance.instance.data.get_timezone())]},
            {"SSID", to_string(config::instance.instance.data.get_wifi_ssid())},
            {"Web user name", to_string(config::instance.instance.data.get_web_user_name())},
            {"Screen brightness (%)", to_string((100 * config::instance.instance.data.get_manual_screen_brightness().value_or(0)) / 256)},
        };
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
    return wifi_manager::instance.get_wifi_status();
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

    // Wire is already used by touch i2c
    if (!Wire1.begin(SDAWire, SCLWire))
    {
        log_e("Failed to begin I2C interface");
        return false;
    }

    scan_i2c_bus();

    if (!temp_hum_sensor.begin(sht31_i2c_address))
    {
        log_e("Failed to start SHT31");
        // return false;
    }
    else
    {
        log_e("SHT31 Initialized");
    }

    temp_hum_sensor.heater(false);

    return true;
}

void hardware::set_sensor_value(sensor_id_index index, const std::optional<sensor_value::value_type> &value)
{
    const auto i = static_cast<size_t>(index);
    if (value.has_value())
    {
        (*sensors_history)[i].add_value(value.value());
        sensors[i].set_value(value.value());
    }
    else
    {
        log_w("Got an invalid value for sensor:%d", index);
        (*sensors_history)[i].clear();
        sensors[i].set_invalid_value();
    }
}

void hardware::begin()
{
    display_instance.begin();
    sdcard::instance.begin();

    sensor_refresh_task = std::make_unique<task_wrapper>([this]
                                                         {
                                                            log_i("Hardware task started on core:%d", xPortGetCoreID());
                                                            do
                                                            {
                                                                read_temperature_humdity_sensor();
                                                                vTaskDelay(sensor_history::sensor_interval/2);
                                                            } while(true); });

    lvgl_refresh_task = std::make_unique<task_wrapper>([this]
                                                       {
                                                            log_i("Lvgl task started on core:%d", xPortGetCoreID());
                                                            do
                                                            {
                                                                display_instance.loop();                                                                                                                         
                                                                vTaskDelay(3);
                                                            } while(true); });

    // start on core 0
    lvgl_refresh_task->spawn_arduino_other_core("lvgl task", 4196);
    sensor_refresh_task->spawn_arduino_other_core("sensor task", 4196);
}

void hardware::read_temperature_humdity_sensor()
{
    const auto now = millis();
    if (now - sht31_sensor_last_read >= sensor_history::sensor_interval)
    {
        sht31_sensor_last_read = now;
        log_i("Reading SHT31 sensor");

        float temperature;
        float humidity;
        temp_hum_sensor.readBoth(&temperature, &humidity);

        set_sensor_value(sensor_id_index::temperatureF, round_value((temperature * 9) / 5 + 32));
        set_sensor_value(sensor_id_index::humidity, round_value(humidity));
    }
}

std::optional<sensor_value::value_type> hardware::round_value(float val, int places)
{
    if (!isnan(val))
    {
        const auto expVal = places == 0 ? 1 : pow(10, places);
        const auto result = float(uint64_t(expVal * val + 0.5)) / expVal;
        return static_cast<sensor_value::value_type>(result);
    }

    return std::nullopt;
}

void hardware::scan_i2c_bus()
{
    log_d("Scanning...");

    auto nDevices = 0;
    for (auto address = 1; address < 127; address++)
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire1.beginTransmission(address);
        auto error = Wire1.endTransmission();

        if (error == 0)
        {
            log_i("I2C device found at address 0x%x", address);
            nDevices++;
        }
    }
    if (nDevices == 0)
    {
        log_i("No I2C devices found\n");
    }
}