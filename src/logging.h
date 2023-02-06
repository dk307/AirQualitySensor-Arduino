#pragma once

#include <deque>

class logger
{
public:
    bool enable_sd_logging();
    bool disable_sd_logging();
    static logger instance;

private:
    void hookUartLogger();
    static void default_ets_logging(const char *message);

    

};