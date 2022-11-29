#pragma once

#include "change_callback.h"
#include <ArduinoJson.h>
#include <mutex>

struct config_data
{

    config_data()
    {
        setDefaults();
    }

    void setDefaults()
    {
        std::lock_guard<std::mutex> lock(data_mutex);

        const auto defaultUserIDPassword = F("admin");
        host_name.clear();
        web_user_name = defaultUserIDPassword;
        web_password = defaultUserIDPassword;
        wifi_ssid.clear();
        wifi_password.clear();
    }

    String get_host_name() const
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        return host_name;
    }
    void set_host_name(const String &host_name_)
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        host_name = host_name_;
    }

    String get_web_user_name() const
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        return web_user_name;
    }
    void set_web_user_name(const String &web_user_name_)
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        web_user_name = web_user_name_;
    }

    String get_web_password() const
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        return web_password;
    }
    void set_web_password(const String &web_password_)
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        web_password = web_password_;
    }

    String get_wifi_ssid() const
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        return wifi_ssid;
    }
    void set_wifi_ssid(const String &wifi_ssid_)
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        wifi_ssid = wifi_ssid_;
    }

    String get_wifi_password() const
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        return wifi_password;
    }
    void set_wifi_password(const String &wifi_password_)
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        wifi_password = wifi_password_;
    }

private:
    String host_name;
    String web_user_name;
    String web_password;
    String wifi_ssid;
    String wifi_password;

    mutable std::mutex data_mutex;
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
    static size_t write_to_file(const String &fileName, const char * data, unsigned int len);

    template <class T>
    bool deserialize_to_json(const T &data, DynamicJsonDocument &jsonDocument);

    bool request_save{false};
};
