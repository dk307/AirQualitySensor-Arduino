#pragma once

#include <WString.h>
#include <vector>
#include <string>
#include <psram_allocator.h>

class ui_interface
{
public:
    typedef std::vector<std::pair<String, String>, psram::allocator<std::pair<String, String>>> information_table_type;
    information_table_type get_information_table();
    static ui_interface instance;

private:
    ui_interface() = default;
    static String get_up_time();
    static String network_status();
};