#include "wifi_manager.h"
#include "config_manager.h"
#include "operations.h"

#include <WiFi.h>

#include <memory>

wifi_manager wifi_manager::instance;

void wifi_manager::begin()
{
    wifi_start();
}

void wifi_manager::wifi_start()
{
    const auto ssid = config::instance.data.get_wifi_ssid();

    bool connected = false;
    if (!ssid.isEmpty())
    {
        const auto rfc_name = get_rfc_name();
        WiFi.setHostname(rfc_name.c_str());

        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), config::instance.data.get_web_password().c_str());

        if (WiFi.waitForConnectResult(timeout) == WL_CONNECTED)
        {
            // connected
            log_i("Connected to stored WiFi details with IP: %s", WiFi.localIP().toString().c_str());
            WiFi.setHostname(rfc_name.c_str());
            WiFi.setAutoReconnect(true);
            connected = true;
        }
    }

    if (!connected)
    {
        start_captive_portal();
    }
}

// void wifi_manager::disconnect(bool disconnectWifi)
// {
//     WiFi.disconnect(disconnectWifi);
// }

// // function to forget current WiFi details and start a captive portal
// void wifi_manager::forget()
// {
//     disconnect(false);
//     startCaptivePortal();

//     log_i("Requested to forget WiFi. Started Captive portal.");
// }

// function to request a connection to new WiFi credentials
// void wifi_manager::setNewWifi(const String &newSSID, const String &newPass)
// {
//     ssid = newSSID;
//     pass = newPass;
//     reconnect = true;
// }

// function to connect to new WiFi credentials
void wifi_manager::set_wifi(const String &newSSID, const String &newPass)
{

    const auto rfc_name = get_rfc_name();
    WiFi.setHostname(rfc_name.c_str());

    // fix for auto connect racing issue
    if (!(WiFi.status() == WL_CONNECTED && (WiFi.SSID() == newSSID)))
    {
        // store old data in case new network is wrong
        const String oldSSID = WiFi.SSID();
        const String oldPSK = WiFi.psk();

        WiFi.begin(newSSID.c_str(), newPass.c_str(), 0, NULL, true);
        vTaskDelay(2000);

        if (WiFi.waitForConnectResult(timeout) != WL_CONNECTED)
        {
            log_e("New connection unsuccessful");
            if (!in_captive_portal)
            {
                WiFi.begin(oldSSID.c_str(), oldPSK.c_str(), 0, NULL, true);
                if (WiFi.waitForConnectResult(timeout) != WL_CONNECTED)
                {
                    log_e("Reconnection failed too");
                    start_captive_portal();
                }
                else
                {
                    log_i("Connected to new WiFi details with IP: %s", WiFi.localIP().toString().c_str());
                    WiFi.setHostname(rfc_name.c_str());
                    WiFi.setAutoReconnect(true);
                }
            }
        }
        else
        {
            if (in_captive_portal)
            {
                stop_captive_portal();
            }

            log_i("New connection successful with %s", WiFi.localIP().toString().c_str());
        }
    };
}

void wifi_manager::start_captive_portal()
{
    const auto rfc_name = get_rfc_name();
    log_i("Opening a captive portal with AP :%s", rfc_name.c_str());

    WiFi.persistent(false);
    // disconnect sta, start ap
    WiFi.disconnect(); //  this alone is not enough to stop the autoconnecter
    WiFi.mode(WIFI_AP);

    WiFi.softAP(rfc_name.c_str());

    dns_server = std::make_unique<DNSServer>();

    /* Setup the DNS server redirecting all the domains to the apIP */
    dns_server->setErrorReplyCode(DNSReplyCode::NoError);
    dns_server->start(53, F("*"), WiFi.softAPIP());

    captive_portal_start = millis();
    in_captive_portal = true;
}

void wifi_manager::stop_captive_portal()
{
    WiFi.mode(WIFI_STA);
    dns_server.reset();

    in_captive_portal = false;
    callChangeListeners();
}

// // return captive portal state
// bool wifi_manager::isCaptivePortal()
// {
//         return inCaptivePortal;
// }

// return current SSID
IPAddress wifi_manager::LocalIP()
{
    return WiFi.localIP();
}

String wifi_manager::SSID()
{
    return WiFi.SSID();
}

int8_t wifi_manager::RSSI()
{
    return WiFi.RSSI();
}

// captive portal loop
void wifi_manager::loop()
{
    if (in_captive_portal)
    {
        dns_server->processNextRequest();

        // only wait for 5 min in portal and then reboot
        if ((millis() - captive_portal_start) > (5 * 60 * 1000))
        { 
            log_i("Captive portal timeout");
            operations::instance.reboot();
        }
    }
}

String wifi_manager::get_rfc_952_host_name(const String &name)
{
    const int MaxLength = 24;
    String rfc952Hostname;
    rfc952Hostname.reserve(MaxLength);

    for (auto &&c : name)
    {
        if (isalnum(c) || c == '-')
        {
            rfc952Hostname += c;
        }
        if (rfc952Hostname.length() >= MaxLength)
        {
            break;
        }
    }

    // remove last -
    size_t i = rfc952Hostname.length() - 1;
    while (rfc952Hostname[i] == '-' && i > 0)
    {
        i--;
    }

    return rfc952Hostname.substring(0, i);
}

String wifi_manager::get_rfc_name()
{
    auto rfc_name = config::instance.data.get_host_name();
    rfc_name.trim();

    if (rfc_name.isEmpty())
    {
        uint64_t chipId = 0;
        for (auto i = 0; i < 17; i = i + 8)
        {
            chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
        }
        rfc_name = "ESP-" + String(chipId, HEX);
    }

    return get_rfc_952_host_name(rfc_name);
}
