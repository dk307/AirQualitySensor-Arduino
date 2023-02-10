#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class sempaphore_lockable final
{
public:
    sempaphore_lockable() : handle(xSemaphoreCreateMutexStatic(&buffer))
    {
    }

    ~sempaphore_lockable()
    {
        vSemaphoreDelete(handle);
    }

    explicit sempaphore_lockable(const sempaphore_lockable &other) = delete;
    sempaphore_lockable &operator=(const sempaphore_lockable &other) = delete;

    void lock()
    {
        xSemaphoreTake(handle, portMAX_DELAY);
    }

    bool try_lock()
    {
        return xSemaphoreTake(handle, 0) == pdTRUE;
    }

    void unlock()
    {
        xSemaphoreGive(handle);
    }

private:
    SemaphoreHandle_t handle;
    StaticSemaphore_t buffer;
};
