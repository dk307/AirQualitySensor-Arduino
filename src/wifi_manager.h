#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <DNSServer.h>
#include <memory>

#include "change_callback.h"

class wifi_manager : public change_callback
{
public:
    void begin();
    void loop();

    // void forget();
    // bool isCaptivePortal();
    void set_wifi(const String &newSSID, const String &newPass);

    static IPAddress LocalIP();
    static String SSID();
    static int8_t RSSI();
    void disconnect(bool disconnectWifi);
    static wifi_manager instance;

private:
    wifi_manager() = default;
    std::unique_ptr<DNSServer> dns_server;

    bool in_captive_portal{false};
    uint64_t captive_portal_start{0};

    const unsigned long timeout = 60000;

    void wifi_start();
    void start_captive_portal();
    void stop_captive_portal();

    static String get_rfc_name();
    static String get_rfc_952_host_name(const String &name);
};
#endif
