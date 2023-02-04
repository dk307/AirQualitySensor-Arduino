#pragma once

#include <AsyncWebServer_ESP32_ENC.h>
#include <FS.h>
#include <vector>

#include <sensor.h>

class web_server
{
public:
    void begin();
    static web_server instance;

private:
    web_server() {}
    void server_routing();

    // handler
    static void handle_login(AsyncWebServerRequest *request);
    static void handle_logout(AsyncWebServerRequest *request);
    static void wifi_update(AsyncWebServerRequest *request);
    static void web_login_update(AsyncWebServerRequest *request);
    static void other_settings_update(AsyncWebServerRequest *request);
    static void factory_reset(AsyncWebServerRequest *request);
    static void restart_device(AsyncWebServerRequest *request);

    static void firmware_update_upload(AsyncWebServerRequest *request,
                                       const String &filename,
                                       size_t index,
                                       uint8_t *data,
                                       size_t len,
                                       bool final);
    static void reboot_on_upload_complete(AsyncWebServerRequest *request);

    void restore_configuration_upload(AsyncWebServerRequest *request,
                                      const String &filename,
                                      size_t index,
                                      uint8_t *data,
                                      size_t len,
                                      bool final);

    static void handle_early_update_disconnect();

    // ajax
    static void sensor_get(AsyncWebServerRequest *request);
    static void wifi_get(AsyncWebServerRequest *request);
    static void information_get(AsyncWebServerRequest *request);
    static void config_get(AsyncWebServerRequest *request);

    // helpers
    static bool is_authenticated(AsyncWebServerRequest *request);
    static bool manage_security(AsyncWebServerRequest *request);
    static void handle_not_found(AsyncWebServerRequest *request);
    static void handle_file_read(AsyncWebServerRequest *request);
    static bool is_captive_portal_request(AsyncWebServerRequest *request);
    static void redirect_to_root(AsyncWebServerRequest *request);
    static void handle_error(AsyncWebServerRequest *request, const String &error, int code);
    void on_event_connect(AsyncEventSourceClient *client);
    bool filter_events(AsyncWebServerRequest *request);

    // fs ajax
    static void handle_file_list(AsyncWebServerRequest *request);
    static void handle_file_download(AsyncWebServerRequest *request);
    void handle_file_upload(AsyncWebServerRequest *request,
                            const String &filename,
                            size_t index,
                            uint8_t *data,
                            size_t len,
                            bool final);
    static void handle_file_upload_complete(AsyncWebServerRequest *request);
    static const char *get_content_type(const String &filename);
    static String join_path(const String &part1, const String &part2);

    static bool is_ip(const String &str);
    static String to_string_ip(const IPAddress &ip);
    template <class Array, class K, class T>
    static void add_key_value_object(Array &array, const K &key, const T &value);
    template <class V, class T>
    static void add_to_json_doc(V &doc, T id, float value);
    void notify_sensor_change(sensor_id_index id);

    AsyncWebServer http_server{80};
    AsyncEventSource events{"/events"};
    std::unique_ptr<std::vector<uint8_t>> restore_config_data;
};
