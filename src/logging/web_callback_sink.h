#pragma once

#include "serial_hook_sink.h"

#include <semaphore_lockable.h>
#include <mutex>
#include <functional>
#include <Arduino.h>

#include <static_queue.h>
#include <task_wrapper.h>

class web_callback_sink final : public serial_hook_sink
{
public:
    web_callback_sink(const std::function<void(const String &)> &callbackP) : background_log_task(std::bind(&web_callback_sink::flush_callback, this)),
                                                                              callback(callbackP)
    {
        background_log_task.spawn_same("web_callback_sink", 4096, tskIDLE_PRIORITY);
    }

    ~web_callback_sink()
    {
        background_log_task.kill();
    }

    void log(char c) override
    {
        queue.enqueue(c, portMAX_DELAY);
    }

private:
    void flush_callback()
    {
        String data;
        data.reserve(128);
        while (true)
        {
            char c;
            if (queue.dequeue(c, portMAX_DELAY))
            {
                data.concat(c);
                if (c == '\n')
                {
                    callback(data);
                    data.clear();
                }
            }
        }
    }

private:
    esp32::static_queue<char, 1024> queue;
    esp32::task background_log_task;
    const std::function<void(const String &)> callback;
};
