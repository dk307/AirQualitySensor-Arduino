#pragma once

#include <WiFi.h>

class ntp_time
{
public:
    void begin();
    void loop();

    std::optional<time_t> get_local_time() const;

    static ntp_time instance;

private:
    bool force_reconnect{false};
    String ntp_server;
    bool time_set{false};

    void on_got_ip(WiFiEvent_t event, WiFiEventInfo_t info);
    static void time_is_set(struct timeval *tv);
};