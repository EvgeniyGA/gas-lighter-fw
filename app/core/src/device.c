#include "device.h"
#include "config_service.h"

device_config_t dev_config = {
        .main_freqency_Hz.name = "main_freq.txt",
        .main_freqency_Hz.default_value = 1000,
        .dac_ampl.name = "dac_ampl.txt",
        .dac_ampl.default_value = 12.34
};


uint8_t load_configs(void){
    config_save_float(
        dev_config.dac_ampl.name,
        &dev_config.dac_ampl.default_value,
        sizeof(dev_config.dac_ampl.value), 
        1
    );

    config_load_float(
        dev_config.dac_ampl.name,
        &dev_config.dac_ampl.value, 
        sizeof(dev_config.dac_ampl.value), 
        1
    );
    return 0;
}