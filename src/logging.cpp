#include "logging.h"

#include <vector>
#include <mutex>

#include <SD.h>
#include <task_wrapper.h>

logger logger::instance;

class SerialHook;
SerialHook *serial_hook_instance = nullptr;

class SerialHook
{
public:
    SerialHook() : write_task(std::bind(&SerialHook::flush_to_disk_task, this))
    {
        fs_buffer.reserve(512);
        serial_hook_instance = this;
        ets_install_putc2(serial_hook);
        write_task.spawn_same("log flush to sd", 4096, tskIDLE_PRIORITY);
    }

    ~SerialHook()
    {
        ets_install_putc2(NULL);
        write_task.kill();
        std::lock_guard<std::mutex> lock(fs_buffer_mutex);
        serial_hook_instance = nullptr;
        _file.close();
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
        std::lock_guard<std::mutex> lock(fs_buffer_mutex);
        fs_buffer.push_back(c);
    }

    void flush_to_disk_task()
    {
        do
        {
            flush_to_disk();
            vTaskDelay(15000);
        } while (true);
    }

    void flush_to_disk()
    {
        {
            std::lock_guard<std::mutex> lock(fs_buffer_mutex);
            if (fs_buffer.empty())
            {
                return;
            }
        }

        bool result = false;

        if (!_file)
        {
            _file = SD.open(_name, "a");
        }
        if (_file && _maxFiles > 1 && shouldRotate(_file))
        {
            auto rn = strlen(_name) + 1 + 3 + 1;
            String a, b;
            a.reserve(rn);
            b.reserve(rn);
            _file.close();
            for (auto i = _maxFiles - 2; i >= 0; i--)
            {
                a = _name;
                if (i)
                {
                    a.concat('.');
                    a.concat(i);
                }
                if (SD.exists(a))
                {
                    b = _name;
                    b.concat('.');
                    b.concat(i + 1);
                    if (SD.exists(b))
                    {
                        SD.remove(b);
                    }
                    SD.rename(a, b);
                }
            }
            _file = SD.open(_name, "a");
        }
        if (_file)
        {
            std::lock_guard<std::mutex> lock(fs_buffer_mutex);
            _file.write(reinterpret_cast<uint8_t *>(fs_buffer.data()), fs_buffer.size()); // ignore error
            _file.flush();
            fs_buffer.clear();
        }
    }

    bool shouldRotate(File &f) { return f.size() > 1024 * 1024; }

    File _file;
    const char *_name{"/logs/log.txt"};
    const uint8_t _maxFiles = 5;

    std::mutex fs_buffer_mutex;
    std::vector<char> fs_buffer;
    task_wrapper write_task;
};

bool logger::enable_sd_logging()
{
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

    hookUartLogger();
    return true;
}

bool logger::disable_sd_logging()
{
    if (serial_hook_instance)
    {
        delete serial_hook_instance;
        serial_hook_instance = nullptr;
    }
}

void logger::hookUartLogger()
{
    if (serial_hook_instance)
    {
        delete serial_hook_instance;
    }
    serial_hook_instance = new SerialHook();
}
