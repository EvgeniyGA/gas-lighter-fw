#pragma once

#include <cstdint>
#include <array>
#include "config_service.h"

template <typename T>
struct TypeInfo{
    using BaseType = T;
    static constexpr size_t ElementSize = sizeof(T);
    static constexpr size_t ElementCount = 1;
};

template <typename T, size_t N>
struct TypeInfo<std::array<T, N>>{
    using BaseType = T;
    static constexpr size_t ElementSize = sizeof(T);
    static constexpr size_t ElementCount = N;
};

template <typename T>
struct ConfigItem{
    const char* name;
    const T def_value;
    T value;
    ConfigItem(const char* param_name, T default_value) : 
        name(param_name), def_value(default_value){};

    T load(){
        T value = def_value;
        config_data_type_e type = std::is_same_v<typename TypeInfo<T>::BaseType, float> ? CONFIG_DATA_TYPE_FLOAT : CONFIG_DATA_TYPE_INT;
        uint8_t res = config_load_raw(
            name,
            reinterpret_cast<uint8_t*>(&value), 
            TypeInfo<T>::ElementSize, 
            TypeInfo<T>::ElementCount,
            type
        );
        if(res != 0){
            save(def_value);
        }
        return value;
    }

    void save(const T val){
        config_data_type_e type = std::is_same_v<typename TypeInfo<T>::BaseType, float> ? CONFIG_DATA_TYPE_FLOAT : CONFIG_DATA_TYPE_INT;
        value = val;
        config_save_raw(
            name,
            reinterpret_cast<uint8_t*>(&value),
            TypeInfo<T>::ElementSize,
            TypeInfo<T>::ElementCount,
            type
        );
    }
};

struct DeviceConfig{
    ConfigItem<uint32_t> main_freq{"freq.txt", 444u};
    ConfigItem<float> dac_set{"dac.txt", 32.23f};
    ConfigItem<std::array<int, 6>> calibration{"calibr.txt", {1u, 2u, 3u, 4u}};
};

