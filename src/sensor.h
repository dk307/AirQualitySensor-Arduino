#pragma once

#include <change_callback.h>
#include <math.h>
#include <atomic>
#include <mutex>
#include <semaphore_lockable.h>
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

template <class T>
class sensor_value_t : public change_callback
{
public:
    typedef T value_type;

    std::optional<value_type> get_value() const
    {
        return value.load();
    }

    void set_value(T value_)
    {
        set_value_(value_);
    }

    void set_invalid_value()
    {
        set_value_(std::nullopt);
    }

private:
    std::atomic<std::optional<T>> value;

    void set_value_(std::optional<value_type> value_)
    {
        if (value.exchange(value_) != value_)
        {
            call_change_listeners();
        }
    }
};

using sensor_value = sensor_value_t<int16_t>;

template <class T, uint16_t countT>
class sensor_history_t
{
public:
    typedef struct
    {
        T mean;
        T min;
        T max;
    } stats;

    using vector_history_t = std::vector<T, esp32::psram::allocator<T>>;

    typedef struct
    {
        std::optional<stats> stat;
        vector_history_t history;
    } sensor_history_snapshot;

    void add_value(T value)
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        last_x_values.push(value);
    }

    void clear()
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        last_x_values.clear();
    }

    sensor_history_snapshot get_snapshot(uint8_t group_by_count) const
    {
        vector_history_t return_values;

        std::lock_guard<esp32::semaphore> lock(data_mutex);
        const auto size = last_x_values.size();
        if (size)
        {
            return_values.reserve(1 + (size / group_by_count));
            stats stats_value{0, std::numeric_limits<T>::max(), std::numeric_limits<T>::min()};
            double sum = 0;
            double group_sum = 0;
            for (auto i = 0; i < size; i++)
            {
                const auto value = last_x_values[i];
                sum += value;
                group_sum += value;
                stats_value.max = std::max(value, stats_value.max);
                stats_value.min = std::min(value, stats_value.min);

                if ((i % group_by_count) == 0)
                {
                    return_values.push_back(group_sum / group_by_count);
                    group_sum = 0;
                }
            }

            // add partial group average
            if (size % group_by_count)
            {
                return_values.push_back(group_sum / (size % group_by_count));
            }

            stats_value.mean = static_cast<T>(sum / size);
            return {stats_value, return_values};
        }
        else
        {
            return {std::nullopt, return_values};
        };
    }
   
    std::optional<T> get_average() const
    {
        std::lock_guard<esp32::semaphore> lock(data_mutex);
        const auto size = last_x_values.size();
        if (size)
        {
            double sum = 0;
            for (auto i = 0; i < size; i++)
            {
                sum += last_x_values[i];
            }
            return static_cast<T>(sum / size);
        }
        else
        {
            return std::nullopt;
        }
    }

private:
    mutable esp32::semaphore data_mutex;
    CircularBuffer<T, countT> last_x_values;
};

template <class T, uint8_t reads_per_minuteT, uint16_t minutesT>
class sensor_history_minute_t : public sensor_history_t<T, reads_per_minuteT * minutesT>
{
public:
    static constexpr auto total_minutes = minutesT;
    static constexpr auto reads_per_minute = reads_per_minuteT;
    static constexpr auto sensor_interval = (60 * 1000 / reads_per_minute);
};

using sensor_history = sensor_history_minute_t<sensor_value::value_type, 12, 240>;

const sensor_definition &get_sensor_definition(sensor_id_index id);
const char *get_sensor_name(sensor_id_index id);
const char *get_sensor_unit(sensor_id_index id);
