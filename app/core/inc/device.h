#pragma once

#include <stdint.h>

#define DEVICE_CONFIG_MAS_MAX   (100)

typedef struct{
    char* name;
    uint32_t default_value;
    uint32_t value;
}device_param_int_t;

typedef struct{
    char* name;
    uint32_t default_value[DEVICE_CONFIG_MAS_MAX];
    uint32_t value[DEVICE_CONFIG_MAS_MAX];
}device_param_int_mas_t;

typedef struct{
    char* name;
    float default_value;
    float value;
}device_param_float_t;

typedef struct{
    device_param_int_t main_freqency_Hz;
    device_param_float_t dac_ampl;
    device_param_int_t calibration;
}device_config_t;

uint8_t load_configs(void);