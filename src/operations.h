#pragma once

#include <Arduino.h>
#include <atomic>

class MultiResetDetector;

class operations
{
public:
    void reboot();
    void begin();
    void loop();

    void factory_reset();

    // update
    bool start_update(size_t length, const String &md5, String &error);
    bool write_update(const uint8_t *data, size_t length, String &error);
    bool end_update(String &error);
    void abort_update();
    bool is_update_in_progress();

    static operations instance;

private:
    operations() {}
    void get_update_error(String &error);
    [[noreturn]] static void reset();

    MultiResetDetector *mrd = nullptr;

    std::atomic_bool reboot_pending{false};
};
