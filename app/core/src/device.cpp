#include "device.h"
#include "config_service.h"
#include <cstdio>
#include "config_wrapper.hpp"
#include <algorithm>

struct DeviceConfig{
    ConfigItem<uint32_t> main_freq{"freq.txt", 444u};
    ConfigItem<float> dac_set{"dac.txt", 32.23f};
    ConfigItem<std::array<int, 10>> calibration{"cal.txt", make_sequence_array<int, 10>()};

};

DeviceConfig dev_config;

uint8_t load_configs(void){
    if(dev_config.main_freq.load() == true){
        printf("freq loaded from file: %d\n\r", static_cast<int>(dev_config.main_freq.value));
        dev_config.main_freq.save(dev_config.main_freq.value + 1);
    }
    else{
        printf("freq use default\n\r");
    }

    if(dev_config.calibration.load() == true){
        printf("calibration:\n\r");
        for(auto& elem: dev_config.calibration.value){
            printf(" %d\n\r", elem);
            elem++;
        }
        dev_config.calibration.save(dev_config.calibration.value);//dev_config.calibration.save({9, 8, 7, 6, 5, 4});
    }
    else{
        printf("use default calibration\n\r");
    }

    return 0;
}


