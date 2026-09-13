#pragma once

#include <cstdint>
#include <array>
#include "config_service.h"

template <typename T>
struct ConfigTypeInfo{
    using base_type = T;
    static constexpr size_t element_count = 1;
    static constexpr size_t element_size = sizeof(T);
};

template <typename T, size_t N>
struct ConfigTypeInfo<std::array<T, N>>{
    using base_type = T;
    static constexpr size_t element_count = N;
    static constexpr size_t element_size = sizeof(T);
};




template <typename T>
struct ConfigItem{
    const char* param_name;
    const T default_value;
    T current_value;

    constexpr ConfigItem(const char* name, T def_val) 
        : param_name(name), default_value(def_val), current_value{} {};

    void load(){
        static_assert(std::is_trivially_copyable_v<T>, "wrong type");//???
        T value = default_value;

        using TypeInfo = ConfigTypeInfo<T>;

        config_data_type_e type = CONFIG_DATA_TYPE_INT;
        if constexpr (std::is_same_v<typename TypeInfo::base_type, float>){
            type = CONFIG_DATA_TYPE_FLOAT;
        }
        
        config_load_raw(
            name,
            reinterpret_cast<uint8_t*>(&value), 
            TypeInfo::element_size, 
            TypeInfo::element_count,
            type
        );

        return value;

    }


};

struct DeviceConfig{
    ConfigItem<uint32_t> main_freq{"freq.txt", 444u};
    ConfigItem<float> dac_set{"dac.txt", 32.23f};
    ConfigItem<std::array<int, 6>> calibration{"calibr.txt", {1u, 2u, 3u, 4u}};
};

