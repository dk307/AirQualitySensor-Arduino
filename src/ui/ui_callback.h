#pragma once

#include <vector>
#include <string>

class ui_callback
{
public:
    std::vector<std::pair<std::string, std::string>> get_information_table();

    static ui_callback instance;

private:
    ui_callback() = default;
};