#pragma once

#include "change_callback.h"
#include <ArduinoJson.h>
#include <atomic>
#include <mutex>
#include <semaphore_lockable.h>

enum class TimeZoneSupported
{
    USEastern = 0,
    USCentral = 1,
    USMountainTime = 2,
    USArizona = 3,
    USPacific = 4
};

struct config_data
{
    config_data()
    {
        setDefaults();
    }

    void setDefaults()
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);

        const auto defaultUserIDPassword = "admin";
        host_name.clear();
        web_user_name = defaultUserIDPassword;
        web_password = defaultUserIDPassword;
        wifi_ssid.clear();
        wifi_password.clear();
        ntp_server.clear();
        ntp_server_refresh_interval = 60 * 60 * 1000;
        time_zone = TimeZoneSupported::USPacific;
        manual_screen_brightness.reset();
    }

    String get_host_name() const
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        return host_name;
    }
    void set_host_name(const String &host_name_)
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        host_name = host_name_;
    }

    String get_web_user_name() const
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        return web_user_name;
    }
    void set_web_user_name(const String &web_user_name_)
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        web_user_name = web_user_name_;
    }

    String get_web_password() const
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        return web_password;
    }
    void set_web_password(const String &web_password_)
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        web_password = web_password_;
    }

    String get_wifi_ssid() const
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        return wifi_ssid;
    }
    void set_wifi_ssid(const String &wifi_ssid_)
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        wifi_ssid = wifi_ssid_;
    }

    String get_wifi_password() const
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        return wifi_password;
    }
    void set_wifi_password(const String &wifi_password_)
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        wifi_password = wifi_password_;
    }

    std::optional<uint8_t> get_manual_screen_brightness() const
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        return manual_screen_brightness;
    }
    void set_manual_screen_brightness(const std::optional<uint8_t> &screen_brightness_)
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        manual_screen_brightness = screen_brightness_;
    }

    String get_ntp_server() const
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        return ntp_server;
    }
    void set_ntp_server(const String &ntp_server_)
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        ntp_server = ntp_server_;
    }

    uint64_t get_ntp_server_refresh_interval() const
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        return ntp_server_refresh_interval;
    }
    void set_ntp_server_refresh_interval(uint64_t ntp_server_refresh_interval_)
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        ntp_server_refresh_interval = ntp_server_refresh_interval_;
    }

    TimeZoneSupported get_timezone() const
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        return time_zone;
    }
    void set_timezone(TimeZoneSupported time_zone_)
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        time_zone = time_zone_;
    }

private:
    String host_name;
    String web_user_name;
    String web_password;
    String wifi_ssid;
    String wifi_password;

    String ntp_server;
    uint64_t ntp_server_refresh_interval;
    TimeZoneSupported time_zone;

    std::optional<uint8_t> manual_screen_brightness;

    mutable esp32::semaphore data_mutex;
};

class config : public change_callback
{
public:
    bool pre_begin();
    void save();
    void reset();
    void loop();

    static void erase();
    static config instance;

    String get_all_config_as_json();

    // does not restore to memory, needs reboot
    bool restore_all_config_as_json(const std::vector<uint8_t> &json, const String &md5);

    config_data data;

private:
    config() = default;
    static String read_file(const String &fileName);

    template <class... T>
    static String md5_hash(T &&...data);

    template <class... T>
    static size_t write_to_file(const String &fileName, T &&...contents);
    static size_t write_to_file(const String &fileName, const char *data, unsigned int len);

    template <class T, class JDoc>
    bool deserialize_to_json(const T &data, JDoc &jsonDocument);

    void save_config();

    std::atomic_bool request_save{false};
};
