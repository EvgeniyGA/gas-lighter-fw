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

namespace device{

DeviceConfig config;

void wave_measure_data_ready_callback(waveMeasureFFT_result_t* result);
void pulse_measure_data_ready_callback(pulse_measure_msg_t* data);

waveGenConfig_s wave_gen_config = {
	.freq = MAIN_FREQENCY_HZ,
	.fun = arm_cos_f32,
	.numb_of_steps = MAIN_TIME_RESOLUTION
};

waveMeasureConfig_s wave_measure_config = {
	.main_freqency = MAIN_FREQENCY_HZ,
	.time_resolution = MAIN_TIME_RESOLUTION,
	.data_ready = wave_measure_data_ready_callback
};

pulseMeasureConfig_s pulse_measure_config = {
	.data_ready = pulse_measure_data_ready_callback,
	.event_half_adc = [](){ gpio_channel_change_state(GPIO_CHANNEL_2a, GPIO_CHANNEL_ON); },
	.event_full_adc = [](){ gpio_channel_change_state(GPIO_CHANNEL_2a, GPIO_CHANNEL_OFF); }
};


void pulse_measure_data_ready_callback(pulse_measure_msg_t* data){
	printf("result %04ld:%04ld:%04ld:%04ld\n\r", 
		data->result[Pulse_Measure_ADC_Channel_1], 
		data->result[Pulse_Measure_ADC_Channel_2], 
		data->result[Pulse_Measure_ADC_Channel_3], 
		data->result[Pulse_Measure_ADC_Channel_4]);
	lcd_print(LCD_PRINTER_LINE1, LCD_PRINTER_OFFSET_ZERO, "%04ld:%04ld %04ld:%04ld\n\r", 
		data->result[Pulse_Measure_ADC_Channel_1], data->result[Pulse_Measure_ADC_Channel_2], 
		data->result[Pulse_Measure_ADC_Channel_3], data->result[Pulse_Measure_ADC_Channel_4]);
	if(data->result[Pulse_Measure_ADC_Channel_2] != 0){
		lcd_print(LCD_PRINTER_LINE2,  LCD_PRINTER_OFFSET_ZERO, "%03ld.%05ld", 
			(uint32_t)(data->result[Pulse_Measure_ADC_Channel_1] / data->result[Pulse_Measure_ADC_Channel_2]), //todo: to float
			((uint32_t)(data->result[Pulse_Measure_ADC_Channel_1] % data->result[Pulse_Measure_ADC_Channel_2])*100000)/data->result[Pulse_Measure_ADC_Channel_2]);
	}
	else{
		lcd_print(LCD_PRINTER_LINE2,  LCD_PRINTER_OFFSET_ZERO, "ch2 zero ");
	}
	if(data->result[Pulse_Measure_ADC_Channel_4] != 0){
		lcd_print(LCD_PRINTER_LINE2, LCD_PRINTER_OFFSET_HALF, "%03ld.%05ld", 
			(uint32_t)(data->result[Pulse_Measure_ADC_Channel_3] / data->result[Pulse_Measure_ADC_Channel_4]), 
			((uint32_t)(data->result[Pulse_Measure_ADC_Channel_3] % data->result[Pulse_Measure_ADC_Channel_4])*100000)/data->result[Pulse_Measure_ADC_Channel_4]);
	}
	else{
		lcd_print(LCD_PRINTER_LINE2,  LCD_PRINTER_OFFSET_ZERO, "ch4 zero ");
	}
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

void init(void){
  	lcd_printer_init();
  	lcd_print(LCD_PRINTER_LINE1, LCD_PRINTER_OFFSET_ZERO + 1, "Version: %s", FW_VERSION_STR);

	wave_measure_init(&wave_measure_config);
	wave_starter_init(&wave_gen_config);
	wave_starter_run(&wave_gen_config);
	pulse_measure_init(&pulse_measure_config);

	if(puse_measure_set_divider(config.pdiv.value) == 0){
		std::printf("pulse period divider %ld setted\r\n", config.pdiv.value);
	}
	else{
		std::printf("ERROR! pulse period divider wrong value\r\n");
	}

	if(wave_starter_set_divider(config.sdiv.value) == 0){
		std::printf("adc period divider %ld setted\r\n", config.sdiv.value);
	}else {
		std::printf("ERROR! adc period divider wrong\r\n");
	}

	if(true){//todo
		std::printf("device init finished:\t\t\t OK\n\r");
	}
	else{
		std::printf("device init finished: \t\t\t FALSE\n\r");
	}
}

}
