#include "device.h"
#include "config_service.h"
#include <cstdio>
#include "config_wrapper.hpp"

DeviceConfig dev_config;

uint8_t load_configs(void){
    printf("freq loaded: %d \n\r", (int)dev_config.main_freq.load());
    dev_config.main_freq.save(4321);
    std::array<int, 6> readed_mas = dev_config.calibration.load();
    for(const auto& elem: readed_mas){
        printf(" %d", elem);
    }
//    dev_config.calibration.save(std::array<int, 6>{9,8,7});
    return 0;
}


device_config_t device_config = {
    .main_freqency_Hz = {
        .name = "freq.txt",
        .default_value = 1000
    },
    .dac_ampl = {
        .name = "dac.txt",
        .default_value = 1.23f
    }

};

uint16_t set_val_int = 777;
float set_val_float = 12.34f;
uint8_t load_configs_(void){
    config_save_raw(
        device_config.main_freqency_Hz.name,
        (uint8_t*)&set_val_int,
        sizeof(device_config.main_freqency_Hz.value), 
        1,
        CONFIG_DATA_TYPE_INT
    );

    config_load_raw(
        device_config.main_freqency_Hz.name,
        (uint8_t*)&device_config.main_freqency_Hz.value, 
        sizeof(device_config.main_freqency_Hz.value), 
        1,
        CONFIG_DATA_TYPE_INT
    );

    config_save_raw(
        device_config.dac_ampl.name,
        (uint8_t*)&set_val_float,
        sizeof(device_config.dac_ampl.value), 
        1,
        CONFIG_DATA_TYPE_FLOAT
    );

    config_load_raw(
        device_config.dac_ampl.name,
        (uint8_t*)&device_config.dac_ampl.value, 
        sizeof(device_config.dac_ampl.value), 
        1,
        CONFIG_DATA_TYPE_FLOAT
    );

    std::printf("ldd %d\n\r", device_config.main_freqency_Hz.value);
    return 0;
}

