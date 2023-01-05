#pragma once

#include "ui_screen.h"
#include "sensor.h"

class ui_screen_with_sensor_panel : public ui_screen
{
public:
    using ui_screen::ui_screen;

    typedef struct
    {
        lv_obj_t *panel{nullptr};
        lv_obj_t *label{nullptr};
    } panel_and_label;

    void init() override
    {
        ui_screen::init();
        set_default_screen();
    }

protected:
    const static uint8_t no_value_label = std::numeric_limits<uint8_t>::max();

    static void set_default_value_in_panel(const panel_and_label &pair)
    {
        if (pair.panel)
        {
            set_label_panel_color(pair.panel, no_value_label);
        }

        if (pair.label)
        {
            lv_label_set_text_static(pair.label, "-");
        }
    }

    static void set_label_panel_color(lv_obj_t *panel, uint8_t level)
    {
        uint32_t color;
        uint32_t color_grad;

        switch (level)
        {
        default:
        case 0:
            color = 0x4BD175; // green
            color_grad = 0x228D44;
            break;
        case 1:
            color = 0x767C30; // yellow
            color_grad = 0xBFCC20;
            break;
        case 2:
            color = 0xEC9706; // orange
            color_grad = 0xED7117;
            break;
        case 3:
            color = 0xE3242B; // red
            color_grad = 0x900D09;
            break;
        case 4:
            color = 0x710193; // purple
            color_grad = 0xA32CC4;
            break;
        case 5:
            color = 0x940606; // Maroon
            color_grad = 0xc30808;
            break;
        case no_value_label:
            color = 0xC0C0C0; // Silver
            color_grad = 0x696969;
            break;
        }

        lv_obj_set_style_bg_color(panel, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(panel, lv_color_hex(color_grad), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    static void set_value_in_panel(const panel_and_label &pair, sensor_id_index index, const std::optional<sensor_value::value_type> &value)
    {
        if (value.has_value())
        {
            const auto final_value = value.value();
            const auto level = sensor_definitions[static_cast<uint8_t>(index)].calculate_level(final_value);
            if (pair.panel)
            {
                set_label_panel_color(pair.panel, level);
            }

            if (pair.label)
            {
                if (!pair.panel)
                {
                    lv_label_set_text_fmt(pair.label, "%d%s", final_value, sensor_definitions[static_cast<uint8_t>(index)].get_unit());
                }
                else
                {
                    lv_label_set_text_fmt(pair.label, "%d", final_value);
                }
            }
        }
        else
        {
            set_default_value_in_panel(pair);
        }
    }
};
