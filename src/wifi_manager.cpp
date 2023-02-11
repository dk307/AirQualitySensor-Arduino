#include "wifi_manager.h"
#include "config_manager.h"
#include "operations.h"

#include <WiFi.h>
#include <StreamString.h>

#include <memory>

wifi_manager wifi_manager::instance;

void wifi_manager::begin()
{
    WiFi.persistent(false);
    WiFi.onEvent(std::bind(&wifi_manager::wifi_event, this, std::placeholders::_1, std::placeholders::_2));
    // WiFi.setAutoReconnect(false);
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_MODE_STA);
    delay(100);
    wifi_start();
}

void wifi_manager::wifi_start()
{
    if (!connect_saved_wifi())
    {
        start_captive_portal();
    }
}

void wifi_manager::set_new_wifi(const String &newSSID, const String &newPass)
{
    log_i("Trying up setup new wifi:%s pwd:%s", newSSID.c_str(), newPass.c_str());
    new_ssid = newSSID;
    new_password = newPass;
    connect_new_ssid = true;
}

bool wifi_manager::connect_saved_wifi()
{
    return connect_wifi(config::instance.data.get_wifi_ssid(), config::instance.data.get_wifi_password());
}

bool wifi_manager::connect_wifi(const String &ssid, const String &password)
{
    if (ssid.isEmpty() || password.isEmpty())
    {
        return false;
    }
    const auto rfc_name = get_rfc_name();
    WiFi.setHostname(rfc_name.c_str());

    log_i("wifi connection:%s pwd:%s", ssid.c_str(), password.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());

    constexpr unsigned long timeout = 30000;
    if (WiFi.waitForConnectResult(timeout) == WL_CONNECTED)
    {
        // connected
        log_i("Connected to WiFi %s with IP: %s", ssid.c_str(), WiFi.localIP().toString().c_str());
        return true;
    }

    return false;
}

// function to connect to new WiFi credentials
void wifi_manager::set_wifi(const String &newSSID, const String &newPass)
{
    // fix for auto connect racing issue
    if (!(WiFi.status() == WL_CONNECTED && (WiFi.SSID() == newSSID)))
    {
        // store old data in case new network is wrong
        const String oldSSID = WiFi.SSID();
        const String oldPSK = WiFi.psk();

        const bool connected = connect_wifi(newSSID, newPass);
        if (!connected)
        {
            log_e("New connection unsuccessful for %s", newSSID.c_str());
            if (!in_captive_portal)
            {
                const bool connect_old = connect_wifi(oldSSID, oldPSK);
                if (!connect_old)
                {
                    start_captive_portal();
                }
            }
            else
            {
                start_captive_portal();
            }
        }
        else
        {
            log_i("Connected to new WiFi details with IP: %s", WiFi.localIP().toString().c_str());

            config::instance.data.set_wifi_ssid(newSSID);
            config::instance.data.set_wifi_password(newPass);
            stop_captive_portal_if_running();
            config::instance.save();
        }
    };
}

void wifi_manager::start_captive_portal()
{
    const auto rfc_name = get_rfc_name();
    log_i("Opening a captive portal with AP :%s", rfc_name.c_str());

    const auto mode = WiFi.getMode();

    WiFi.mode(WIFI_AP_STA);
    delay(100);
    WiFi.softAP(rfc_name.c_str());

    dns_server = esp32::psram::make_unique<DNSServer>();

    /* Setup the DNS server redirecting all the domains to the apIP */
    dns_server->setErrorReplyCode(DNSReplyCode::NoError);
    dns_server->start(53, F("*"), WiFi.softAPIP());

    captive_portal_start = millis();
    in_captive_portal = true;
    call_change_listeners();
}

void wifi_manager::stop_captive_portal_if_running()
{
    if (in_captive_portal)
    {
        log_i("Stopping captive portal");
        dns_server.reset();

        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_MODE_STA);
        delay(100);

        in_captive_portal = false;
        call_change_listeners();
    }
}

bool wifi_manager::is_captive_portal()
{
    return in_captive_portal;
}

// return current SSID
IPAddress wifi_manager::get_local_ip()
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

void wifi_manager::loop()
{
    const uint32_t max_captive_portal_time = 5 * 60 * 1000;
    const uint8_t max_connection_retries = 10;
    const uint32_t connection_retry_interval = 60 * 1000;

    const auto now = millis();

    if (in_captive_portal)
    {
        dns_server->processNextRequest();

        // only wait for 5 min in portal and then reboot
        if ((now - captive_portal_start) > max_captive_portal_time)
        {
            log_i("Captive portal timeout. Restarting");
            operations::instance.reboot();
            return;
        }
    }

    if (connect_new_ssid)
    {
        set_wifi(new_ssid, new_password);
        reconnect_retries = 0;
        reconnect_last_retry = millis();
        connect_new_ssid = false;
        new_ssid.clear();
        new_password.clear();
    }

    if (!in_captive_portal)
    {
        // check every connection_retry_interval
        if ((now - reconnect_last_retry >= connection_retry_interval) || check_connection)
        {
            check_connection = false;
            if (!WiFi.isConnected())
            {
                if (reconnect_retries <= max_connection_retries)
                {
                    log_w("Disconnected from wifi, connection retry no:%d", reconnect_retries);
                    if (!connect_saved_wifi())
                    {
                        log_w("Connection to saved wifi failed for retry no:%d", reconnect_retries);
                    }
                    else
                    {
                        log_i("Connection to saved wifi succeeded for retry no:%d", reconnect_retries);
                    }
                    reconnect_retries++;
                    reconnect_last_retry = millis(); // get the time again to account for time taken to connect to wifi
                }
                else
                {
                    start_captive_portal();
                }
            }
            else
            {
                // valid connection for connection_retry_interval
                if ((now - reconnect_last_retry >= connection_retry_interval) && reconnect_retries)
                {
                    log_i("Wifi connection is stable now");
                    reconnect_retries = 0;
                }
            }
        }
    }
}

String wifi_manager::get_rfc_952_host_name(const String &name)
{
    const int max_length = 24;
    String rfc_952_hostname;
    rfc_952_hostname.reserve(max_length);

    for (auto &&c : name)
    {
        if (isalnum(c) || c == '-')
        {
            rfc_952_hostname += c;
        }
        if (rfc_952_hostname.length() >= max_length)
        {
            break;
        }
    }

    // remove last -
    size_t i = rfc_952_hostname.length() - 1;
    while (rfc_952_hostname[i] == '-' && i > 0)
    {
        i--;
    }

    return rfc_952_hostname.substring(0, i);
}

String wifi_manager::get_rfc_name()
{
    auto rfc_name = config::instance.data.get_host_name();
    rfc_name.trim();

    if (rfc_name.isEmpty())
    {
        const auto mac_address = ESP.getEfuseMac();
        uint64_t chipId = 0;
        for (auto i = 0; i < 17; i = i + 8)
        {
            chipId |= ((mac_address >> (40 - i)) & 0xff) << i;
        }
        rfc_name = "ESP-" + String(chipId, HEX);
    }

    return get_rfc_952_host_name(rfc_name);
}

void wifi_manager::wifi_event(arduino_event_id_t event, arduino_event_info_t info)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_AP_START:
    case ARDUINO_EVENT_WIFI_AP_STOP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        call_change_listeners();
        break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        log_d("WiFi STA disconnected");
        check_connection = true;
        call_change_listeners();
        break;
    }
}

bool wifi_manager::is_wifi_connected()
{
    log_d("Captive Portal:%d", in_captive_portal);
    log_d("Mode:%d", WiFi.getMode());
    log_d("SSID:%s", WiFi.SSID().c_str());
    log_d("Status:%d", WiFi.status());
    log_d("IP:%s", WiFi.localIP().toString().c_str());

    if (!in_captive_portal)
    {
        const auto mode = WiFi.getMode();
        if ((mode == WIFI_MODE_STA) || (mode == WIFI_MODE_APSTA))
        {
            return (WiFi.status() == WL_CONNECTED) &&
                   static_cast<uint32_t>(WiFi.localIP()) != 0;
        }
    }
    return false;
}

String wifi_manager::get_wifi_status()
{
    StreamString stream;
    if (!in_captive_portal)
    {
        const auto mode = WiFi.getMode();
        if ((mode == WIFI_MODE_STA) || (mode == WIFI_MODE_APSTA))
        {
            if ((WiFi.status() == WL_CONNECTED) &&
                static_cast<uint32_t>(WiFi.localIP()) != 0)
            {
                stream.printf("Connected to %s with IP %s", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
            }
            else
            {
                stream.printf("Not connected to Wifi");
            }
        }
        else
        {
            stream.printf("Access Point with SSID:%s", WiFi.softAPSSID().c_str());
        }
    }
    else
    {
        if (connect_new_ssid)
        {
            stream.printf("Connecting to %s", new_ssid.c_str());
        }
        else
        {
            stream.printf("Access Point with SSID:%s", WiFi.softAPSSID().c_str());
        }
    }
    return stream;
}