#include "config_manager.h"

#include <Arduino.h>

#include <LittleFS.h>
#include <MD5Builder.h>

static const char ConfigFilePath[] PROGMEM = "/Config.json";
static const char ConfigChecksumFilePath[] PROGMEM = "/ConfigChecksum.json";
static const char HostNameId[] PROGMEM = "hostname";
static const char WebUserNameId[] PROGMEM = "webusername";
static const char WebPasswordId[] PROGMEM = "webpassword";


config config::instance;

template <class... T>
String config::md5Hash(T &&...data)
{
    MD5Builder hashBuilder;
    hashBuilder.begin();
    hashBuilder.add(data...);
    hashBuilder.calculate();
    return hashBuilder.toString();
}

template <class... T>
size_t config::writeToFile(const String &fileName, T &&...contents)
{
    File file = LittleFS.open(fileName, "w");
    if (!file)
    {
        return 0;
    }

    const auto bytesWritten = file.write(contents...);
    file.close();
    return bytesWritten;
}

void config::erase()
{
    LittleFS.remove(FPSTR(ConfigChecksumFilePath));
    LittleFS.remove(FPSTR(ConfigFilePath));
}

bool config::begin()
{
    const auto configData = readFile(FPSTR(ConfigFilePath));

    if (configData.isEmpty())
    {
        log_w("No stored config found");
        reset();
        return false;
    }

    DynamicJsonDocument jsonDocument(2048);
    if (!deserializeToJson(configData.c_str(), jsonDocument))
    {
        reset();
        return false;
    }

    // read checksum from file
    const auto readChecksum = readFile(FPSTR(ConfigChecksumFilePath));
    const auto checksum = md5Hash(configData);

    if (!checksum.equalsIgnoreCase(readChecksum))
    {
        log_e("Config data checksum mismatch");
        reset();
        return false;
    }

    data.set_host_name(jsonDocument[FPSTR(HostNameId)].as<String>());
    // data.webUserName = jsonDocument[FPSTR(WebUserNameId)].as<String>();
    // data.webPassword = jsonDocument[FPSTR(WebPasswordId)].as<String>();

    // data.ntpServer1 = jsonDocument[FPSTR(NtpServer1Id)].as<String>();
    // data.ntpServer2 = jsonDocument[FPSTR(NtpServer2Id)].as<String>();
    // data.timezone = static_cast<TimeZoneSupported>(jsonDocument[FPSTR(TimeZoneId)].as<uint64_t>());
    // data.ntpServerRefreshInterval = jsonDocument[FPSTR(NtpServerRefreshIntervalId)].as<uint64_t>();

    // LOG_DEBUG(F("Loaded Config from file"));
    return true;
}

void config::reset()
{
    data.setDefaults();
    requestSave = true;
}

void config::save()
{
    // LOG_INFO(F("Saving configuration"));

    // DynamicJsonDocument jsonDocument(2048);

    // jsonDocument[FPSTR(HostNameId)] = data.hostName.c_str();
    // jsonDocument[FPSTR(WebUserNameId)] = data.webUserName.c_str();
    // jsonDocument[FPSTR(WebPasswordId)] = data.webPassword.c_str();
    // jsonDocument[FPSTR(NtpServer1Id)] = data.ntpServer1;
    // jsonDocument[FPSTR(NtpServer2Id)] = data.ntpServer2;

    // jsonDocument[FPSTR(NtpServerRefreshIntervalId)] = data.ntpServerRefreshInterval;
    // jsonDocument[FPSTR(TimeZoneId)] = static_cast<uint64_t>(data.timezone);

    // String json;
    // serializeJson(jsonDocument, json);

    // if (writeToFile(FPSTR(ConfigFilePath), json.c_str(), json.length()) == json.length())
    // {
    //     const auto checksum = md5Hash(json);
    //     if (writeToFile(FPSTR(ConfigChecksumFilePath), checksum.c_str(), checksum.length()) != checksum.length())
    //     {
    //         LOG_ERROR(F("Failed to write config checksum file"));
    //     }
    // }
    // else
    // {
    //     LOG_ERROR(F("Failed to write config file"));
    // }

    // LOG_INFO(F("Saving Configuration done"));
    // callChangeListeners();
}

void config::loop()
{
    if (requestSave)
    {
        requestSave = false;
        save();
    }
}

String config::readFile(const String &fileName)
{
    File file = LittleFS.open(fileName, "r");
    if (!file)
    {
        return String();
    }

    const auto json = file.readString();
    file.close();
    return json;
}

String config::getAllConfigAsJson()
{
    loop(); // save if needed
    return readFile(FPSTR(ConfigFilePath));
}

bool config::restoreAllConfigAsJson(const std::vector<uint8_t> &json, const String &hashMd5)
{
    // DynamicJsonDocument jsonDocument(2048);
    // if (!deserializeToJson(json, jsonDocument))
    // {
    //     return false;
    // }

    // const auto expectedMd5 = md5Hash(json.data(), json.size());
    // if (!expectedMd5.equalsIgnoreCase(hashMd5))
    // {
    //     LOG_ERROR(F("Uploaded Md5 for config does not match. File md5:") << expectedMd5);
    //     return false;
    // }

    // if (writeToFile(FPSTR(ConfigFilePath), json.data(), json.size()) != json.size())
    // {
    //     return false;
    // }

    // if (writeToFile(FPSTR(ConfigChecksumFilePath), hashMd5.c_str(), hashMd5.length()) != hashMd5.length())
    // {
    //     return false;
    // }
    return true;
}

template <class T>
bool config::deserializeToJson(const T &data, DynamicJsonDocument &jsonDocument)
{
    DeserializationError error = deserializeJson(jsonDocument, data);

    // Test if parsing succeeds.
    if (error)
    {
        log_e("deserializeJson for config failed: %s" , error.f_str());
        return false;
    }
    return true;
}
