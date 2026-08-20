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
#include "lcd.h"
#include "main.h"
#include "queue.h"
#include "lcd_printer.h"

#define STORAGE_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define BLINKY_STACK_SIZE   configMINIMAL_STACK_SIZE

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTOTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

#define PRINTER_MESSAGE_LEN   36
typedef struct{
  uint8_t* data;
  uint32_t dataLen;
}printerMessage_t;

QueueHandle_t print_queue;

void led_blinking_task(void* param);
void print_task(void* param);

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
LCD_HandleTypeDef hlcd1;

void setup(void){
    hlcd1.RS_Port = disp_a0_GPIO_Port; hlcd1.RS_Pin = disp_a0_Pin;
    hlcd1.EN_Port = disp_e_GPIO_Port; hlcd1.EN_Pin = disp_e_Pin;
    hlcd1.D4_Port = disp_d4_GPIO_Port; hlcd1.D4_Pin = disp_d4_Pin;
    hlcd1.D5_Port = disp_d5_GPIO_Port; hlcd1.D5_Pin = disp_d5_Pin;
    hlcd1.D6_Port = disp_d6_GPIO_Port; hlcd1.D6_Pin = disp_d6_Pin;
    hlcd1.D7_Port = disp_d7_GPIO_Port; hlcd1.D7_Pin = disp_d7_Pin;

  LCD_Init(&hlcd1);
  LCD_Clear(&hlcd1);

  LCD_SetCursor(&hlcd1, 0, 3); // Строка 1, 4-й символ
  LCD_SendString(&hlcd1, "Hello, STM32F4!");
  LCD_SetCursor(&hlcd1, 1, 2); // Строка 2, 3-й символ
  LCD_SendString(&hlcd1, "MT-20S4A 20x4");

	printf("Firmware version: %s\n", FW_VERSION_STR);
	printf("Build: %s %s (git: %s)\n", FW_BUILD_DATE, FW_BUILD_TIME, FW_GIT_HASH);
	printf("Version: %d.%d.%d\n", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH);

	if (is_hash_invalid(FW_GIT_HASH)) {
		printf("ERROR: Invalid firmware hash detected: %s\r\n", FW_GIT_HASH ? FW_GIT_HASH : "NULL");
	} else {
		printf("FW Hash: %s\r\n", FW_GIT_HASH);
	}

 // print_queue = NULL;
 // print_queue = xQueueCreate(PRINTER_MESSAGE_LEN, sizeof(printerMessage_t));
  
 // if(print_queue == NULL){
 //   while(1);
 // }

  lcd_printer_init();

	wave_measure_config.main_freqency = MAIN_FREQENCY_HZ;
	wave_measure_config.time_resolution = MAIN_TIME_RESOLUTION;
	wave_measure_init(&wave_measure_config);

	wave_gen_config.freq = MAIN_FREQENCY_HZ;
	wave_gen_config.numb_of_steps = MAIN_TIME_RESOLUTION;
	wave_gen_config.fun = arm_cos_f32;
	wave_starter_init(&wave_gen_config);
	wave_starter_run(&wave_gen_config);

	pulse_measure_init(&pulse_measure_config);

	FATFS_Init();
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
  static uint32_t i;
  while (1) {
    SEGGER_SYSVIEW_PrintfHost("BlikTask started");
    vTaskDelay(blink_interval_ms / portTICK_PERIOD_MS);
    led_state = 1 - led_state; // toggle
//	  printf("blink %04d\n\r", i++);
//    lcd_print(1, "blink %03d", i++);
  }
}
