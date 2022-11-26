#pragma once

#include <atomic>
#include <functional>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class task_wrapper
{
public:
    task_wrapper(std::function<void(void)> call) : call(call)
    {
    }
    ~task_wrapper() { kill(); }

    TaskHandle_t handle() const { return handle_.load(); }

    /// Starts the task without specifying CPU affinity.
    esp_err_t spawn(const char *name, uint32_t stack_depth, uint32_t priority)
    {
        kill();
        TaskHandle_t handle = nullptr;
        if (xTaskCreate(RunAdapter, name, stack_depth, this, priority, &handle) != pdTRUE)
        {
            return ESP_ERR_NO_MEM;
        }
        handle_.store(handle);
        return ESP_OK;
    }

    esp_err_t spawn_arduino_main_core(const char *name, uint32_t stack_depth = 8192, uint32_t priority = 1)
    {
        return spawn_pinned(name, stack_depth, priority, 1);
    }

    /// Starts the task on the specified CPU.
    esp_err_t spawn_pinned(const char *name, uint32_t stack_depth, uint32_t priority, BaseType_t cpu)
    {
        kill();
        TaskHandle_t handle = nullptr;
        if (xTaskCreatePinnedToCore(RunAdapter, name, stack_depth, this, priority, &handle, cpu) !=
            pdTRUE)
        {
            return ESP_ERR_NO_MEM;
        }
        handle_.store(handle);
        return ESP_OK;
    }

    /// Starts the task on the same CPU as the caller.
    esp_err_t spawn_same(const char *name, uint32_t stack_depth, uint32_t priority)
    {
        return spawn_pinned(name, stack_depth, priority, xPortGetCoreID());
    }

    /// Stops the task if not already stopped.
    void kill()
    {
        const TaskHandle_t handle = handle_.exchange(nullptr);
        if (handle)
        {
            vTaskDelete(handle);
        }
    }

protected:
    void run()
    {
        call();
    }

private:
    std::function<void(void)> call;
    std::atomic<TaskHandle_t> handle_{nullptr};

    task_wrapper(const task_wrapper &) = delete;
    task_wrapper(task_wrapper &&) = delete;
    task_wrapper &operator=(const task_wrapper &) = delete;

    static void RunAdapter(void *self) { reinterpret_cast<task_wrapper *>(self)->run(); }
};
