#pragma once

#include <vector>
#include <functional>

class change_callback
{
public:
    void addConfigSaveCallback(std::function<void()> func)
    {
        configsavecallback.push_back(func);
    }

    void callChangeListeners() const
    {
        for (auto &&ftn : configsavecallback)
        {
            ftn();
        }
    }

private:
    std::vector<std::function<void()>> configsavecallback;
};