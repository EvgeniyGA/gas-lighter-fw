#include "device.h"
#include "config_service.h"
#include <cstdio>
#include "config_wrapper.hpp"
#include <algorithm>

struct DeviceConfig{
    ConfigItem<uint32_t> main_freq{"freq.txt", 444u};
    ConfigItem<float> dac_set{"dac.txt", 32.23f};
    ConfigItem<std::array<int, 6>> calibration{"calibr.txt", {1, 2, 3, 4}};
};

DeviceConfig dev_config;

uint8_t load_configs(void){
    auto freq = dev_config.main_freq.load();
    if(freq.from_file == true){
        printf("freq loaded from file: %d\n\r", static_cast<int>(freq.value));
        dev_config.main_freq.save(freq.value + 1);
    }
    else{
        printf("freq use default");
    }

    auto calibr = dev_config.calibration.load();
    if(calibr.from_file == true){
        printf("calibration:\n\r");
        for(auto& elem: calibr.value){
            printf(" %d\n\r", elem);
            elem++;
        }
        dev_config.calibration.save(calibr.value);//dev_config.calibration.save({9, 8, 7, 6, 5, 4});
    }
    else{
        printf("use default calibration\n\r");
    }

    return 0;
}


