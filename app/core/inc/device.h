#pragma once

#include <stdint.h>

typedef struct{
    char* name;
    uint32_t default_value;
    uint32_t value;
}device_param_int_t;

typedef struct{
    const char* name;
    float default_value;
    float value;
}device_param_float_t;

typedef struct{
    device_param_int_t main_freqency_Hz;
    device_param_float_t dac_ampl;
}device_config_t;

uint8_t load_configs(void);