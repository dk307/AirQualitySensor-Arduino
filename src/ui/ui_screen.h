#pragma once

#include <lvgl.h>
#include <WString.h>

class ui_screen
{
public:
    virtual void init()
    {
        screen = lv_obj_create(NULL);
    }

    void load_screen(lv_scr_load_anim_t anim_type = LV_SCR_LOAD_ANIM_NONE, uint32_t time = 0, uint32_t delay = 0)
    {
        lv_scr_load_anim(screen, anim_type, time, delay, false);
    }

protected:
    lv_obj_t *screen{nullptr};
};
 