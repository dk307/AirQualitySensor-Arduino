#include "logging.h"

#include <vector>
#include <SD.h>
#include <task_wrapper.h>

#include "serial_hook_sink.h"

logger logger::instance;

class SerialHook;
SerialHook *serial_hook_instance = nullptr;

// class web_callback_sink final : public serial_hook_sink
// {
// public:
//     web_callback_sink() : background_callback_task(std::bind(&SerialHook::callback_log_task, this))
//     {
//     }

//     std::atomic_bool enable_callback_logging{false};
//     esp32::semaphore callback_buffer_mutex;
//     std::vector<char> callback_buffer;
//     task_wrapper background_callback_task;
//     std::function<void(const String &log)> logging_callback;
// };

class SerialHook
{
public:
    SerialHook()
    {
        serial_hook_instance = this;
        ets_install_putc2(serial_hook);
    }

    ~SerialHook()
    {
        ets_install_putc2(NULL);
        serial_hook_instance = nullptr;
    }

    void add_sink(serial_hook_sink *sink)
    {
        std::lock_guard<esp32::semaphore> lock(sinks_mutex);
        sinks.push_back(sink);
    }

    void remove_sink(serial_hook_sink *sink)
    {
        std::lock_guard<esp32::semaphore> lock(sinks_mutex);
        sinks.push_back(sink);
    }

private:
    static void serial_hook(char c)
    {
        auto h = serial_hook_instance;
        if (h)
        {
            h->hookImpl(c);
        }
    }

    void hookImpl(char c)
    {
        std::lock_guard<esp32::semaphore> lock(sinks_mutex);
        for (auto &&sink : sinks)
        {
            if (sink)
            {
                sink->log(c);
            }
        }
    }

    esp32::semaphore sinks_mutex;
    std::vector<serial_hook_sink *> sinks;
};

bool logger::enable_sd_logging()
{
    std::lock_guard<esp32::semaphore> lock(serial_hook_mutex);
    // ensure logs dir
    const auto logDir = "/logs";
    if (!SD.exists(logDir))
    {
        if (!SD.mkdir("/logs"))
        {
            log_e("Failed to create logs directory");
            return false;
        }
    }

    hook_uart_logger();

    if (sd_card_sink_instance)
    {
        serial_hook_instance->remove_sink(sd_card_sink_instance.get());
        sd_card_sink_instance.reset();
    }

    sd_card_sink_instance = std::make_unique<sd_card_sink>();
    serial_hook_instance->add_sink(sd_card_sink_instance.get());

    return true;
}

void logger::disable_sd_logging()
{
    std::lock_guard<esp32::semaphore> lock(serial_hook_mutex);

    if (sd_card_sink_instance)
    {
        serial_hook_instance->remove_sink(sd_card_sink_instance.get());
        sd_card_sink_instance.reset();
    }

    if (serial_hook_instance)
    {
        delete serial_hook_instance;
        serial_hook_instance = nullptr;
    }
}

void logger::hook_uart_logger()
{
    if (!serial_hook_instance)
    {
        serial_hook_instance = new SerialHook();
    }
}
