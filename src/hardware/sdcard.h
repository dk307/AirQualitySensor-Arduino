#pragma once

class sdcard
{
public:
    bool pre_begin();
    void begin();
    void loop();

    static sdcard instance;

    bool mount();

private:
    sdcard() = default;
};