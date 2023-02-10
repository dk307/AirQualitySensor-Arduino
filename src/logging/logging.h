#pragma once

#include <sempaphore_lockable.h>

#include "sd_card_sink.h"

class logger
{
public:
    bool enable_sd_logging();
    void disable_sd_logging();

    static logger instance;

private:
    std::mutex serial_hook_mutex;
    std::unique_ptr<sd_card_sink> sd_card_sink_instance;
    
    void hook_uart_logger();
    static void default_ets_logging(const char *message);
};