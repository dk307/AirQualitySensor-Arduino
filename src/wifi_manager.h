#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <DNSServer.h>
#include <memory>
#include <psram_allocator.h>

#include "change_callback.h"

class wifi_manager : public change_callback
{
public:
    void begin();
    void loop();

    // void forget();
    bool is_captive_portal();
    void set_new_wifi(const String &newSSID, const String &newPass);

    static IPAddress LocalIP();
    static String SSID();
    static int8_t RSSI();
    static wifi_manager instance;

private:
    wifi_manager() = default;
    std::unique_ptr<DNSServer, psram::deleter> dns_server;

    bool reconnect = false;
    String newSsid;
    String newPassword;

    bool in_captive_portal{false};
    uint64_t captive_portal_start{0};

    void wifi_start();
    void start_captive_portal();
    void stop_captive_portal();
    void set_wifi(const String &newSSID, const String &newPass);
    static bool connect_wifi(const String &newSSID, const String &newPass);

    static String get_rfc_name();
    static String get_rfc_952_host_name(const String &name);
};
#endif
