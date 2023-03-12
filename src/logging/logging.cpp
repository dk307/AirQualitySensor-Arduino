#include "logging.h"
#include "logging_tags.h"

#include <vector>
#include <SD.h>
#include <task_wrapper.h>

logger logger::instance;

class SerialHook
{
public:
    SerialHook()
    {
        logger::instance.serial_hook_instance = this;
        ets_install_putc2(serial_hook);
        ESP_LOGI(LOGGING_TAG, "Hooked Logging");
    }

    ~SerialHook()
    {
        ets_install_putc2(NULL);
        logger::instance.serial_hook_instance = nullptr;
        ESP_LOGI(LOGGING_TAG, "Removed Logging Hook");
    }

    void add_sink(serial_hook_sink *sink)
    {
        std::lock_guard<esp32::semaphore> lock(sinks_mutex);
        sinks.push_back(sink);
    }

    void remove_sink(serial_hook_sink *sink)
    {
        std::lock_guard<esp32::semaphore> lock(sinks_mutex);
        auto iter = std::find(sinks.begin(), sinks.end(), sink);
        if (iter != sinks.end())
        {
            sinks.erase(iter);
        }
    }

    auto sink_size()
    {
        std::lock_guard<esp32::semaphore> lock(sinks_mutex);
        return sinks.size();
    }

private:
    static void serial_hook(char c)
    {
        auto h = logger::instance.serial_hook_instance;
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

    ESP_LOGI(LOGGING_TAG, "Enabling sd card logging");

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

bool logger::enable_web_logging(const std::function<void(const String &)> &callbackP)
{
    std::lock_guard<esp32::semaphore> lock(serial_hook_mutex);

    ESP_LOGI(LOGGING_TAG, "Enabling web callback card logging");
    hook_uart_logger();

    if (web_callback_sink_instance)
    {
        serial_hook_instance->remove_sink(web_callback_sink_instance.get());
        web_callback_sink_instance.reset();
    }

    web_callback_sink_instance = std::make_unique<web_callback_sink>(callbackP);
    serial_hook_instance->add_sink(web_callback_sink_instance.get());

    return true;
}

template <class T>
void logger::remove_sink(std::unique_ptr<T> &p)
{
    std::lock_guard<esp32::semaphore> lock(serial_hook_mutex);

    if (p)
    {
        serial_hook_instance->remove_sink(p.get());
        p.reset();
    }

    if (serial_hook_instance)
    {
        if (!serial_hook_instance->sink_size())
        {
            delete serial_hook_instance;
            p.reset();
        }
    }
}

void logger::disable_sd_logging()
{
    remove_sink(sd_card_sink_instance);
    ESP_LOGI(LOGGING_TAG, "Disabled sd card logging");
}

void logger::disable_web_logging()
{
    remove_sink(web_callback_sink_instance);
    ESP_LOGI(LOGGING_TAG, "Disabled web callback card logging");
}

void logger::hook_uart_logger()
{
    if (!serial_hook_instance)
    {
        serial_hook_instance = new SerialHook();
    }
}

void logger::set_logging_level(const char *tag, esp_log_level_t level)
{
    ESP_LOGI(LOGGING_TAG, "Setting log level for %s to %d", tag, level);
    esp_log_level_set(tag, level);
}
