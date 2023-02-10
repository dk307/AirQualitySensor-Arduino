#pragma once

class serial_hook_sink
{
public:
    virtual void log(char c) = 0;
    virtual ~serial_hook_sink() = default;
};
