#include "device.h"
#include "config_service.h"
#include <cstdio>
#include "config_wrapper.hpp"
#include <algorithm>

#include "wave_starter.h"
#include "wave_measure.h"
#include "pulse_measure.h"
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

waveGenConfig_s 	wave_gen_config;
waveMeasureConfig_s wave_measure_config;
pulseMeasureConfig_s pulse_measure_config;

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


void pulse_measure_data_ready_callback(pulse_measure_msg_t* data){
	printf("result %04ld:%04ld:%04ld:%04ld\n\r", 
		data->result[Pulse_Measure_ADC_Channel_1], 
		data->result[Pulse_Measure_ADC_Channel_2], 
		data->result[Pulse_Measure_ADC_Channel_3], 
		data->result[Pulse_Measure_ADC_Channel_4]);
	lcd_print(LCD_PRINTER_LINE1, LCD_PRINTER_OFFSET_ZERO, "%04ld:%04ld %04ld:%04ld\n\r", 
		data->result[Pulse_Measure_ADC_Channel_1], data->result[Pulse_Measure_ADC_Channel_2], 
		data->result[Pulse_Measure_ADC_Channel_3], data->result[Pulse_Measure_ADC_Channel_4]);
	lcd_print(LCD_PRINTER_LINE2,  LCD_PRINTER_OFFSET_ZERO, "%03ld.%05ld", 
		(uint32_t)(data->result[Pulse_Measure_ADC_Channel_1] / data->result[Pulse_Measure_ADC_Channel_2]), //todo: to float
		((uint32_t)(data->result[Pulse_Measure_ADC_Channel_1] % data->result[Pulse_Measure_ADC_Channel_2])*100000)/data->result[Pulse_Measure_ADC_Channel_2]);
	lcd_print(LCD_PRINTER_LINE2, LCD_PRINTER_OFFSET_HALF, "%03ld.%05ld", 
		(uint32_t)(data->result[Pulse_Measure_ADC_Channel_3] / data->result[Pulse_Measure_ADC_Channel_4]), 
		((uint32_t)(data->result[Pulse_Measure_ADC_Channel_3] % data->result[Pulse_Measure_ADC_Channel_4])*100000)/data->result[Pulse_Measure_ADC_Channel_4]);
}

void wave_measure_data_ready_callback(waveMeasureFFT_result_t* result){
	lcd_print(LCD_PRINTER_LINE3, 0, "%5d Hz, %5d Hz", 	(int)(result[WAVE_MEASURE_Channel_1].main_freq_Hz), 
										(int)(result[WAVE_MEASURE_Channel_2].main_freq_Hz));
	if(result[WAVE_MEASURE_Channel_1].main_freq_Hz == result[WAVE_MEASURE_Channel_2].main_freq_Hz){
		float diff = result[WAVE_MEASURE_Channel_1].main_phase_deg - result[WAVE_MEASURE_Channel_2].main_phase_deg;
		int diff_x100 = (int)(diff * 100);
		lcd_print(LCD_PRINTER_LINE4, LCD_PRINTER_OFFSET_ZERO, "dPhase: %d.%02d deg", diff_x100/100, abs(diff_x100%100));//todo
	}
	else{
		lcd_print(LCD_PRINTER_LINE4, LCD_PRINTER_OFFSET_ZERO, "                    ");
	}
}

void heartbit_callback(void){

}

void init_device(void){
  	lcd_printer_init();
  	lcd_print(LCD_PRINTER_LINE1, LCD_PRINTER_OFFSET_ZERO + 1, "Version: %s", FW_VERSION_STR);

	wave_measure_config.main_freqency = MAIN_FREQENCY_HZ;
	wave_measure_config.time_resolution = MAIN_TIME_RESOLUTION;
	wave_measure_config.data_ready = wave_measure_data_ready_callback;
	wave_measure_init(&wave_measure_config);

	wave_gen_config.freq = MAIN_FREQENCY_HZ;
	wave_gen_config.numb_of_steps = MAIN_TIME_RESOLUTION;
	wave_gen_config.fun = arm_cos_f32;
	wave_starter_init(&wave_gen_config);
	wave_starter_run(&wave_gen_config);

	pulse_measure_config.data_ready = pulse_measure_data_ready_callback;
	pulse_measure_config.event_half_adc = [](){ gpio_channel_change_state(GPIO_CHANNEL_2a, GPIO_CHANNEL_ON); };
	pulse_measure_config.event_full_adc = [](){ gpio_channel_change_state(GPIO_CHANNEL_2a, GPIO_CHANNEL_OFF); };
	pulse_measure_init(&pulse_measure_config);
}