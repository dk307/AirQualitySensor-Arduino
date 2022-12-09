#pragma once

class sdcard
{
public:
    bool pre_begin();
    void begin();

    static sdcard instance;

private:
    sdcard() = default;

    bool mount();
};