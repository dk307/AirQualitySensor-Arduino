#pragma once

#include <vector>
#include <functional>

class change_callback
{
public:
    void add_config_save_callback(const std::function<void()>& func)
    {
        change_callbacks.push_back(func);
    }

    void call_change_listeners() const
    {
        for (auto &&ftn : change_callbacks)
        {
            ftn();
        }
    }

private:
    std::vector<std::function<void()>> change_callbacks;
};