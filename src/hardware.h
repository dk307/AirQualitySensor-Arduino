#pragma once

#include <SHT31.h>
#include <sps30.h>
#include <BH1750.h>

#include "sensor.h"
#include "task_wrapper.h"
#include "psram_allocator.h"
#include "hardware/display.h"
#include "hardware/sdcard.h"
#include "ui/ui_interface.h"

class hardware final : ui_interface
{
public:
    bool pre_begin();
    void begin();

    static hardware instance;

    const sensor_value &get_sensor(sensor_id_index index) const
    {
        return sensors[static_cast<uint8_t>(index)];
    }

    const sensor_history &get_sensor_history(sensor_id_index index) const
    {
        return (*sensors_history)[static_cast<uint8_t>(index)];
    }

    void update_boot_message(const String &message)
    {
        display_instance.update_boot_message(message);
    }
    void set_main_screen()
    {
        display_instance.set_main_screen();
    }

    // ui_interface
    information_table_type get_information_table(information_type type) override;
    void set_screen_brightness(uint8_t value) override;
    std::optional<sensor_value::value_type> get_sensor_value(sensor_id_index index) const override;
    sensor_history::sensor_history_snapshot get_sensor_detail_info(sensor_id_index index) override;
    bool is_wifi_connected() override;
    String get_wifi_status() override;
    bool clean_sps_30() override;

private:
    hardware() = default;

    display display_instance{*this};
    uint8_t current_brightness{0};

    // same index as sensor_id_index
    std::array<sensor_value, total_sensors> sensors;
    std::unique_ptr<std::array<sensor_history, total_sensors>, esp32::psram::deleter> sensors_history;

    std::unique_ptr<esp32::task> sensor_refresh_task;
    std::unique_ptr<esp32::task> lvgl_refresh_task;

    using light_sensor_values_t = sensor_history_t<sensor_value::value_type, 10>;
    light_sensor_values_t light_sensor_values;

    const int SDAWire = 11;
    const int SCLWire = 10;

    // SHT31
    const int sht31_i2c_address = 0x44;
    SHT31 sht31_sensor;
    int sht31_last_error{0xFF};

    // SPS 30
    uint32_t sps30_sensor_last_read = 0;

    // BH1750
    BH1750 bh1750_sensor;
    uint32_t bh1750_sensor_last_read = 0;

    void set_sensor_value(sensor_id_index index, const std::optional<sensor_value::value_type> &value);

    static String get_up_time();
    void read_bh1750_sensor();
    void read_sht31_sensor();
    void read_sps30_sensor();
    String get_sht31_status();
    String get_sps30_error_register_status();
    uint8_t lux_to_intensity(sensor_value::value_type lux);
    void set_auto_display_brightness();

    static std::optional<sensor_value::value_type> round_value(float val, int places = 0);
    static void scan_i2c_bus();
};
