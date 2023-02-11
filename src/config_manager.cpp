#include "config_manager.h"

#include <Arduino.h>
#include <psram_allocator.h>

#include <SD.h>
#include <MD5Builder.h>

static const char ConfigFilePath[] PROGMEM = "/config.json";
static const char ConfigChecksumFilePath[] PROGMEM = "/config_checksum.json";
static const char HostNameId[] PROGMEM = "hostname";
static const char WebUserNameId[] PROGMEM = "webusername";
static const char WebPasswordId[] PROGMEM = "webpassword";
static const char SsidId[] PROGMEM = "ssid";
static const char SsidPasswordId[] PROGMEM = "ssidpassword";
static const char NtpServerId[] PROGMEM = "ntpserver";
static const char NtpServerRefreshIntervalId[] PROGMEM = "ntpserverrefreshinterval";
static const char TimeZoneId[] PROGMEM = "timezone";
static const char ScreenBrightnessId[] PROGMEM = "screenbrightness";

config config::instance;

template <class... T>
String config::md5_hash(T &&...data)
{
    MD5Builder hashBuilder;
    hashBuilder.begin();
    hashBuilder.add(data...);
    hashBuilder.calculate();
    return hashBuilder.toString();
}

template <class... T>
size_t config::write_to_file(const String &fileName, T &&...contents)
{
    File file = SD.open(fileName, "w");
    if (!file)
    {
        return 0;
    }

    const auto bytesWritten = file.write(contents...);
    file.close();
    return bytesWritten;
}

size_t config::write_to_file(const String &fileName, const char *data, unsigned int len)
{
    return write_to_file(fileName, reinterpret_cast<const uint8_t *>(data), static_cast<size_t>(len));
}

void config::erase()
{
    SD.remove((ConfigChecksumFilePath));
    SD.remove((ConfigFilePath));
}

bool config::pre_begin()
{
    const auto config_data = read_file((ConfigFilePath));

    if (config_data.isEmpty())
    {
        log_w("No stored config found");
        reset();
        return false;
    }

    BasicJsonDocument<esp32::psram::json_allocator> json_document(2048);
    if (!deserialize_to_json(config_data.c_str(), json_document))
    {
        reset();
        return false;
    }

    // read checksum from file
    const auto read_checksum = read_file((ConfigChecksumFilePath));
    const auto checksum = md5_hash(config_data);

    if (!checksum.equalsIgnoreCase(read_checksum))
    {
        log_e("Config data checksum mismatch");
        reset();
        return false;
    }

    data.set_host_name(json_document[(HostNameId)].as<String>());
    data.set_web_user_name(json_document[(WebUserNameId)].as<String>());
    data.set_web_password(json_document[(WebPasswordId)].as<String>());
    data.set_wifi_ssid(json_document[(SsidId)].as<String>());
    data.set_wifi_password(json_document[(SsidPasswordId)].as<String>());
    data.set_ntp_server(json_document[(NtpServerId)].as<String>());
    data.set_timezone(static_cast<TimeZoneSupported>(json_document[(TimeZoneId)].as<uint64_t>()));
    data.set_ntp_server_refresh_interval(json_document[(NtpServerRefreshIntervalId)].as<uint64_t>());

    const auto screen_brightness = json_document[ScreenBrightnessId];
    data.set_manual_screen_brightness(!screen_brightness.isNull() ? std::optional<uint8_t>(screen_brightness.as<uint8_t>()) : std::nullopt);


    log_i("Loaded Config from file");

    log_i("Hostname:%s", data.get_host_name().c_str());
    log_i("Web user name:%s", data.get_web_user_name().c_str());
    log_i("Web user password:%s", data.get_web_password().c_str());
    log_i("Wifi ssid:%s", data.get_wifi_ssid().c_str());
    log_i("Wifi ssid password:%s", data.get_wifi_password().c_str());
    log_i("Manual screen brightness:%d", data.get_manual_screen_brightness().value_or(0));
    log_i("Ntp Server:%s", data.get_ntp_server().c_str());
    log_i("Ntp Server Refresh Interval:%d ms", data.get_ntp_server_refresh_interval());
    log_i("Time zone:%d", data.get_timezone());

    return true;
}

void config::reset()
{
    log_i("config reset is requested");
    data.setDefaults();
    request_save.store(true);
}

void config::save()
{
    log_d("config save is requested");
    request_save.store(true);
}

void config::save_config()
{
    log_i("Saving configuration");

    BasicJsonDocument<esp32::psram::json_allocator> json_document(2048);

    json_document[(HostNameId)] = data.get_host_name();
    json_document[(WebUserNameId)] = data.get_web_user_name();
    json_document[(WebPasswordId)] = data.get_web_password();
    json_document[(SsidId)] = data.get_wifi_ssid();
    json_document[(SsidPasswordId)] = data.get_wifi_password();

    json_document[(NtpServerId)] = data.get_ntp_server();
    json_document[(NtpServerRefreshIntervalId)] = data.get_ntp_server_refresh_interval();
    json_document[(TimeZoneId)] = static_cast<uint64_t>(data.get_timezone());

    const auto brightness = data.get_manual_screen_brightness();
    if (brightness.has_value())
    {
        json_document[ScreenBrightnessId] = brightness.value();
    }
    else
    {
        json_document[ScreenBrightnessId] = nullptr;
    }

    String json;
    serializeJson(json_document, json);

    if (write_to_file((ConfigFilePath), json.c_str(), json.length()) == json.length())
    {
        const auto checksum = md5_hash(json);
        if (write_to_file((ConfigChecksumFilePath), checksum.c_str(), checksum.length()) != checksum.length())
        {
            log_e("Failed to write config checksum file");
        }
    }
    else
    {
        log_e("Failed to write config file");
    }

    log_i("Saving Configuration done");
    call_change_listeners();
}

void config::loop()
{
    bool expected = true;
    if (request_save.compare_exchange_strong(expected, false))
    {
        save_config();
    }
}

String config::read_file(const String &fileName)
{
    File file = SD.open(fileName, "r");
    if (!file)
    {
        return String();
    }

    const auto json = file.readString();
    file.close();
    return json;
}

String config::get_all_config_as_json()
{
    loop(); // save if needed
    return read_file((ConfigFilePath));
}

bool config::restore_all_config_as_json(const std::vector<uint8_t> &json, const String &hashMd5)
{
    BasicJsonDocument<esp32::psram::json_allocator> json_doc(2048);
    if (!deserialize_to_json(json, json_doc))
    {
        return false;
    }

    const auto expected_md5 = md5_hash(const_cast<uint8_t *>(json.data()), json.size());
    if (!expected_md5.equalsIgnoreCase(hashMd5))
    {
        log_e("Uploaded Md5 for config does not match. File md5:%s", expected_md5.c_str());
        return false;
    }

    if (write_to_file((ConfigFilePath), json.data(), json.size()) != json.size())
    {
        return false;
    }

    if (write_to_file((ConfigChecksumFilePath), hashMd5.c_str(), hashMd5.length()) != hashMd5.length())
    {
        return false;
    }
    return true;
}

template <class T, class JDoc>
bool config::deserialize_to_json(const T &data, JDoc &jsonDocument)
{
    DeserializationError error = deserializeJson(jsonDocument, data);

    // Test if parsing succeeds.
    if (error)
    {
        log_e("deserializeJson for config failed: %s", error.f_str());
        return false;
    }
    return true;
}
