#include "device.h"
#include "config_service.h"

device_config_t dev_config = {
        .main_freqency_Hz.name = "freq.txt",
        .main_freqency_Hz.default_value = 1000,
        .dac_ampl.name = "dac_ampl.txt",
        .dac_ampl.default_value = 12.34
};

uint16_t set_val = 321;
uint8_t load_configs(void){
    config_save(
        dev_config.main_freqency_Hz.name,
        (uint8_t*)&set_val,
        sizeof(dev_config.main_freqency_Hz.value), 
        1
    );

    config_load(
        dev_config.main_freqency_Hz.name,
        (uint8_t*)&dev_config.main_freqency_Hz.value, 
        sizeof(dev_config.main_freqency_Hz.value), 
        1
    );
    printf("ldd %d\n\r", dev_config.main_freqency_Hz.value);
    return 0;
}