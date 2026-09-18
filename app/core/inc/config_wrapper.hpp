#pragma once

#include <cstdint>
#include <array>
#include <type_traits>
#include "config_service.h"

//namespace Config
//{

template <typename T, std::size_t... Is>
constexpr std::array<T, sizeof...(Is)> make_sequence_array_impl(std::index_sequence<Is...>) {
    return { static_cast<T>(Is + 1)... };
}

template <typename T, std::size_t N>
constexpr std::array<T, N> make_sequence_array() {
    return make_sequence_array_impl<T>(std::make_index_sequence<N>{});
}

template <typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

template <TriviallyCopyable T>
struct TypeInfo{
    using BaseType = T;
    static constexpr size_t ElementSize = sizeof(T);
    static constexpr size_t ElementCount = 1;
};

template <TriviallyCopyable T, size_t N>
struct TypeInfo<std::array<T, N>>{
    using BaseType = T;
    static constexpr size_t ElementSize = sizeof(T);
    static constexpr size_t ElementCount = N;
};

template <TriviallyCopyable T>
struct ConfigItem{
    const char* name;
    const T def_value;
    T value;
    constexpr ConfigItem(const char* param_name, T default_value) noexcept
        : name(param_name), def_value(default_value){};

    [[nodiscard]] bool load() noexcept {
        constexpr config_data_type_e type = std::is_same_v<typename TypeInfo<T>::BaseType, float> ? CONFIG_DATA_TYPE_FLOAT : CONFIG_DATA_TYPE_INT;
        uint8_t res = config_load_raw(
            name,
            reinterpret_cast<uint8_t*>(&value), 
            TypeInfo<T>::ElementSize, 
            TypeInfo<T>::ElementCount,
            type
        );
        if(res != 0){
            value = def_value;
            save(def_value);
        }
        return (res == 0) ? true : false;
    }

    void save(const T& val) noexcept {
        constexpr config_data_type_e type = std::is_same_v<typename TypeInfo<T>::BaseType, float> ? CONFIG_DATA_TYPE_FLOAT : CONFIG_DATA_TYPE_INT;
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

//}

