#include <stdio.h>
#include <ctype.h>
#include <FreeRTOS.h>
#include "task.h"
#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW.h"
#include "tusb.h"
#include "fatfs.h"
#include "version.h"
#include "version_check.h"
#include "app.h"
#include "arm_math.h"
#include "main.h"
#include "queue.h"
#include "lcd_printer.h"
#include "cli_service.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "gpio_driver.h"
#include "config_service.h"
#include "status_service.h"
#ifndef FOR_QEMU
	#include "wave_starter.h"
	#include "wave_measure.h"
	#include "pulse_measure.h"
	#include "usb_service.h"
#endif

#ifndef FOR_QEMU
waveGenConfig_s 	wave_gen_config;
waveMeasureConfig_s wave_measure_config;
pulseMeasureConfig_s pulse_measure_config;
usb_device_config_t usb_device_config;
#endif

void init(void){
#ifndef FOR_QEMU
	SEGGER_RTT_ConfigUpBuffer( 0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM );
#endif
	SEGGER_SYSVIEW_Conf();
  	SEGGER_SYSVIEW_Start();
  	while(SEGGER_SYSVIEW_IsStarted()==0);
  	SEGGER_RTT_WriteString( 0, "SEGGER Real-Time-Terminal Started\n" );
}

#ifndef FOR_QEMU
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

void pulse_measure_event_half_callback(void){
	gpio_channel_change_state(GPIO_CHANNEL_2a, GPIO_CHANNEL_ON);
}
void pulse_measure_event_full_callback(void){
	gpio_channel_change_state(GPIO_CHANNEL_2a, GPIO_CHANNEL_OFF);
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

void usb_device_mounted_callback(void){
	printf("USB device mounted\n\r");
}

void usb_device_unmounted_callback(void){
	printf("USB device unmounted\n\r");
}
#endif

float for_load[] = {123, 321};
float for_check[2];
float val = 0;
void heartbit_callback(void){
	float loaded_int_val = 0;
	val++;
	config_save_float("mas1", for_load, sizeof(for_load[0]), sizeof(for_load)/sizeof(for_load[0]));
	vTaskDelay(100);
	config_load_float("mas1", for_check, sizeof(for_check[0]), sizeof(for_check)/sizeof(for_check[0]));

	printf("___________\n\r");
	printf("loaded mas: \n\r");
	for(int i = 0; i < sizeof(for_check)/sizeof(for_check[0]); i++){
		printf("%d\n\r", for_check[i]);
	}

	config_save_float("var1", &val, sizeof(val), 1);
	vTaskDelay(100);
	config_load_float("var1", &loaded_int_val, sizeof(loaded_int_val), 1);
	printf("loaded %d\n\r", loaded_int_val);
	lcd_print(LCD_PRINTER_LINE4, LCD_PRINTER_OFFSET_FULL_NEXT - 3, "%3d", loaded_int_val);
}

void setup(void){
	printf("Firmware version: %s\n", FW_VERSION_STR);
	printf("Build: %s %s (git: %s)\n", FW_BUILD_DATE, FW_BUILD_TIME, FW_GIT_HASH);
	printf("Version: %d.%d.%d\n", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH);

	if (is_hash_invalid(FW_GIT_HASH)) {
		printf("ERROR: Invalid firmware hash detected: %s\r\n", FW_GIT_HASH ? FW_GIT_HASH : "NULL");
	} else {
		printf("FW Hash: %s\r\n", FW_GIT_HASH);
	}

  	lcd_printer_init();
  	lcd_print(LCD_PRINTER_LINE1, LCD_PRINTER_OFFSET_ZERO + 1, "Version: %s", FW_VERSION_STR);
#ifndef FOR_QEMU	
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
	pulse_measure_config.event_half_adc = pulse_measure_event_half_callback;
	pulse_measure_config.event_full_adc = pulse_measure_event_full_callback;
	pulse_measure_init(&pulse_measure_config);

	usb_device_config.mounted = usb_device_mounted_callback;
	usb_device_config.unmounted = usb_device_unmounted_callback;
#endif
	FATFS_Init();
	cli_service_init();
	config_service_init();
	status_service_init(heartbit_callback);
#ifndef FOR_QEMU
	usb_device_init(&usb_device_config);
	usb_cdc_init();
#endif
	vTaskStartScheduler();
}


