#pragma once

#include "serial_hook_sink.h"

#include <sempaphore_lockable.h>
#include <mutex>
#include <functional>
#include <Arduino.h>
#include <SD.h>

#include <task_wrapper.h>

class sd_card_sink final : public serial_hook_sink
{
public:
    sd_card_sink() : background_log_task(std::bind(&sd_card_sink::flush_to_disk_task, this))
    {
    }

    ~sd_card_sink()
    {
        background_log_task.kill();
        flush_to_disk();
        sd_card_file.close();
    }

    void log(char c) override
    {
        std::lock_guard<sempaphore_lockable> lock(fs_buffer_mutex);
        fs_buffer.push_back(c);
    }

    void flush_to_disk_task()
    {
        do
        {
            vTaskDelay(15000);
            flush_to_disk();
        } while (true);
    }

    void flush_to_disk()
    {
        {
            std::lock_guard<sempaphore_lockable> lock(fs_buffer_mutex);
            if (fs_buffer.empty())
            {
                return;
            }
        }

        bool result = false;

        if (!sd_card_file)
        {
            sd_card_file = SD.open(sd_card_name, "a");
        }
        if (sd_card_file && sd_card_max_files > 1 && should_rotate(sd_card_file))
        {
            auto rn = strlen(sd_card_name) + 1 + 3 + 1;
            String a, b;
            a.reserve(rn);
            b.reserve(rn);
            sd_card_file.close();
            for (auto i = sd_card_max_files - 2; i >= 0; i--)
            {
                a = sd_card_name;
                if (i)
                {
                    a.concat('.');
                    a.concat(i);
                }
                if (SD.exists(a))
                {
                    b = sd_card_name;
                    b.concat('.');
                    b.concat(i + 1);
                    if (SD.exists(b))
                    {
                        SD.remove(b);
                    }
                    SD.rename(a, b);
                }
            }
            sd_card_file = SD.open(sd_card_name, "a");
        }
        if (sd_card_file)
        {
            std::lock_guard<sempaphore_lockable> lock(fs_buffer_mutex);
            sd_card_file.write(reinterpret_cast<uint8_t *>(fs_buffer.data()), fs_buffer.size()); // ignore error
            sd_card_file.flush();
            fs_buffer.clear();
        }
    }

    bool should_rotate(File &f) { return f.size() > 1024 * 1024; }

private:
    File sd_card_file;
    const char *sd_card_name{"/logs/log.txt"};
    const uint8_t sd_card_max_files = 5;
    sempaphore_lockable fs_buffer_mutex;
    std::vector<char> fs_buffer;
    task_wrapper background_log_task;
};
