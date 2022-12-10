#include "ntp_time.h"

#include "config_manager.h"
#include <TimeLib.h>
#include <Timezone.h>

#include <WiFi.h>
#include <esp_sntp.h>

ntp_time EXT_RAM_ATTR ntp_time::instance;

// US Eastern Time Zone (New York, Detroit)
TimeChangeRule EXT_RAM_ATTR usEDT = {"EDT", Second, Sun, Mar, 2, -240}; // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule EXT_RAM_ATTR usEST = {"EST", First, Sun, Nov, 2, -300};  // Eastern Standard Time = UTC - 5 hours
Timezone EXT_RAM_ATTR usET(usEDT, usEST);

// US Central Time Zone (Chicago, Houston)
TimeChangeRule EXT_RAM_ATTR usCDT = {"CDT", Second, Sun, Mar, 2, -300};
TimeChangeRule EXT_RAM_ATTR usCST = {"CST", First, Sun, Nov, 2, -360};
Timezone EXT_RAM_ATTR usCT(usCDT, usCST);

// US Mountain Time Zone (Denver, Salt Lake City)
TimeChangeRule EXT_RAM_ATTR usMDT = {"MDT", Second, Sun, Mar, 2, -360};
TimeChangeRule EXT_RAM_ATTR usMST = {"MST", First, Sun, Nov, 2, -420};
Timezone EXT_RAM_ATTR usMT(usMDT, usMST);

// Arizona is US Mountain Time Zone but does not use DST
Timezone EXT_RAM_ATTR usAZ(usMST);

// US Pacific Time Zone (Las Vegas, Los Angeles)
TimeChangeRule EXT_RAM_ATTR usPDT = {"PDT", Second, Sun, Mar, 2, -420};
TimeChangeRule EXT_RAM_ATTR usPST = {"PST", First, Sun, Nov, 2, -480};
Timezone EXT_RAM_ATTR usPT(usPDT, usPST);

void ntp_time::begin()
{
    WiFi.onEvent(std::bind(&ntp_time::on_got_ip, this, std::placeholders::_1, std::placeholders::_2), ARDUINO_EVENT_WIFI_STA_GOT_IP);

    log_i("Wifi Status:%d", WiFi.status());
    force_reconnect = true;

    const auto ftn = [this]
    {
        log_d("Config reset, get time again");
        force_reconnect = true;
    };

    config::instance.add_callback(ftn);
    sntp_set_time_sync_notification_cb(time_is_set);
}
 

void ntp_time::time_is_set(struct timeval *tv)
{
    const auto status = sntp_get_sync_status();
    if (!ntp_time::instance.time_set)
    {
        log_i("Time is set");
        ntp_time::instance.time_set = true;
    }
    else
    {
        log_d("Time is updated");
    }
}

void ntp_time::loop()
{
    if (force_reconnect)
    {
        log_i("Doing NTP Setup");
        force_reconnect = false;
        ntp_server = config::instance.data.get_ntp_server();
        sntp_set_sync_interval(config::instance.data.get_ntp_server_refresh_interval());
        configTime(0, 0, ntp_server.c_str());
        sntp_restart();
    }
}

void ntp_time::on_got_ip(WiFiEvent_t event, WiFiEventInfo_t info)
{
    log_i("Got Wifi IP");
    force_reconnect = true;
}

 

// std::optional<timentp::DisplayTime> timentp::getDisplayTime() const
// {
//     if (timeSet)
//     {
//         Timezone*  tz;
//         switch (config::instance.data.timezone)
//         {
//             default:
//             case TimeZoneSupported::USEastern:
//             tz = &usET;
//             break;
//             case TimeZoneSupported::USCentral:
//             tz = &usCT;
//             break;
//             case TimeZoneSupported::USMountainTime:
//             tz = &usMT;
//             break;
//             case TimeZoneSupported::USArizona:
//             tz = &usAZ;
//             break;
//             case TimeZoneSupported::USPacific:
//             tz = &usPT;
//             break;
//         }

//         const auto utc = time(NULL);
//         TimeChangeRule *tcr;
//         const time_t t = tz->toLocal(utc, &tcr);
//         return DisplayTime{hour(t), minute(t)};
//     }
//     return std::nullopt;
// }
