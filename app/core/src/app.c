#include <stdio.h>
#include <ctype.h>
#include <FreeRTOS.h>
#include "task.h"
#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW.h"
#include "tusb.h"
#include "fatfs.h"
#include "usb_service.h"
#include "version.h"
#include "version_check.h"
#include "app.h"
#include "arm_math.h"
#include "wave_starter.h"
#include "wave_measure.h"
#include "pulse_measure.h"
#include "main.h"
#include "queue.h"
#include "lcd_printer.h"
#include "cli_service.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "gpio_driver.h"

#define STORAGE_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define BLINKY_STACK_SIZE   configMINIMAL_STACK_SIZE

void led_blinking_task(void* param);
void print_task(void* param);
void data_manager_task(void* param);

void init(void){
#ifndef FOR_QEMU
	SEGGER_RTT_ConfigUpBuffer( 0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM );
#endif
  SEGGER_SYSVIEW_Conf();
  SEGGER_SYSVIEW_Start();
  while(SEGGER_SYSVIEW_IsStarted()==0);
  SEGGER_RTT_WriteString( 0, "SEGGER Real-Time-Terminal Started\n" );
}

waveGenConfig_s 	wave_gen_config;
waveMeasureConfig_s wave_measure_config;
pulseMeasureConfig_s pulse_measure_config;

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
	pulse_measure_config.led_on = gpio_channel_change_state(GPIO_CHANNEL_2a, GPIO_CHANNEL_ON);
	pulse_measure_config.led_off = gpio_channel_change_state(GPIO_CHANNEL_2a, GPIO_CHANNEL_OFF);
	pulse_measure_init(&pulse_measure_config);

	FATFS_Init();
	cli_service_init();
	xTaskCreate(led_blinking_task, "blinky", BLINKY_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
	xTaskCreate(cdc_task, "cdc", CDC_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);
	vTaskStartScheduler();
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void* param) {
  (void) param;
  static uint8_t led_state = 0;
  while (1) {
    SEGGER_SYSVIEW_PrintfHost("BlikTask started");
    vTaskDelay(250 / portTICK_PERIOD_MS);
    led_state = 1 - led_state; // toggle
//	  printf("blink %04d\n\r", i++);
//    lcd_print(0, 1, "counter: %d", i++);
  }
}
