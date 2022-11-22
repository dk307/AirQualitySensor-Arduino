#pragma once



class hardware
{
public:
    bool pre_begin();
    void begin();
    void loop();

    static hardware instance;

private:
    hardware() = default;
};