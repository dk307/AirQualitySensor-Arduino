#pragma once

#include <vector>
#include <functional>

class change_callback
{
public:
    void add_callback(const std::function<void()>& func) const
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
    mutable std::vector<std::function<void()>> change_callbacks;
};