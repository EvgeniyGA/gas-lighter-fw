#include "device.h"
#include "config_service.h"
#include <cstdio>
#include "config_wrapper.hpp"
#include <algorithm>

#include "gpio_driver.h"
#include "lcd_printer.h"

#include "version.h"
#include "version_check.h"

struct DeviceConfig{
    config::Item<uint32_t> main_freq{"freq.txt", 444u};
    config::Item<float> dac_set{"dac.txt", 32.23f};
    config::Item<std::array<int, 10>> calibration{"cal.txt", config::make_sequence_array<int, 10>()};
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

void heartbit_callback(void){

}

void init_device(void){
  	lcd_printer_init();
  	lcd_print(LCD_PRINTER_LINE1, LCD_PRINTER_OFFSET_ZERO + 1, "Version: %s", FW_VERSION_STR);

}