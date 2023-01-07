#pragma once

#include <change_callback.h>

#include <math.h>
#include <atomic>
#include <mutex>
#include <CircularBuffer.h>
#include <psram_allocator.h>

#include "sensor_id.h"

class sensor_definition_display
{
public:
    sensor_definition_display(double range_min, double range_max, sensor_level level) : range_min(range_min), range_max(range_max), level(level)
    {
    }

    double get_range_min() const { return range_min; }
    double get_range_max() const { return range_max; }
    sensor_level get_level() const { return level; }

    bool is_in_range(double value) const
    {
        return range_min <= value && value < range_max;
    }

private:
    const double range_min;
    const double range_max;
    const sensor_level level;
};

class sensor_definition
{
public:
    sensor_definition(const char *name, const char *unit, const sensor_definition_display *display_definitions, size_t display_definitions_count)
        : name(name), unit(unit), display_definitions(display_definitions), display_definitions_count(display_definitions_count)
    {
    }

    sensor_level calculate_level(double value_p) const
    {
        for (uint8_t i = 0; i < display_definitions_count; i++)
        {
            if (display_definitions[i].is_in_range(value_p))
            {
                return display_definitions[i].get_level();
            }
        }
        return display_definitions[0].get_level();
    }

    const char *get_unit() const { return unit; }
    const char *get_name() const { return name; }

private:
    const char *name;
    const char *unit;
    const sensor_definition_display *display_definitions;
    const uint8_t display_definitions_count;
};

extern const std::array<sensor_definition, total_sensors> sensor_definitions;

template <class T>
class sensor_value_t : public change_callback
{
public:
    typedef T value_type;

    std::optional<value_type> get_value() const
    {
        std::unique_lock<std::mutex> lock(data_mutex);
        return value;
    }

    template <class T1>
    void set_value(T1 value_)
    {
        const auto new_value = static_cast<value_type>(value_);
        set_value_(new_value);
    }

    void set_invalid_value()
    {
        set_value(std::nullopt);
    }

private:
    mutable std::mutex data_mutex;
    std::optional<T> value;

    void set_value_(const std::optional<value_type>& value_)
    {
        std::unique_lock<std::mutex> lock(data_mutex);
        if (value != value_)
        {
            value = value_;
            lock.unlock();
            call_change_listeners();
        }
    }
};

using sensor_value = sensor_value_t<int16_t>;

template <class T, size_t Count>
class sensor_history_t
{
public:
    typedef struct
    {
        T mean;
        T min;
        T max;
    } stats;

    typedef struct
    {
        std::optional<stats> last_x_min_stats;
        std::vector<T, psram::allocator<T>> last_x_min_values;
    } sensor_history_snapshot;

    static constexpr int reads_per_minute = 3;

    void add_value(T value)
    {
        std::lock_guard<std::mutex> lock(data_mutex);
        last_x_min_values.push(value);
    }

    sensor_history_snapshot get_snapshot() const
    {
        std::lock_guard<std::mutex> lock(data_mutex);

        const auto size = last_x_min_values.size();
        std::vector<T, psram::allocator<T>> return_values;
        if (size)
        {
            return_values.reserve(last_x_min_values.size());
            stats stats_value{0, std::numeric_limits<T>::max(), std::numeric_limits<T>::min()};
            double sum = 0;
            for (auto i = 0; i < size; i++)
            {
                const auto value = last_x_min_values[i];
                sum += value;
                stats_value.max = std::max(value, stats_value.max);
                stats_value.min = std::min(value, stats_value.min);
                return_values.push_back(last_x_min_values[i]);
            }
            stats_value.mean = static_cast<T>(sum / size);
            return {stats_value, return_values};
        }
        else
        {
            return {std::nullopt, return_values};
        }
    }

private:
    mutable std::mutex data_mutex;
    CircularBuffer<T, reads_per_minute * Count> last_x_min_values;
};

using sensor_history = sensor_history_t<sensor_value::value_type, 240>;
