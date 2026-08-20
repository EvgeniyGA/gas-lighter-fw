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



void setup(void){
	printf("Firmware version: %s\n", FW_VERSION_STR);
  printf("Build: %s %s (git: %s)\n", FW_BUILD_DATE, FW_BUILD_TIME, FW_GIT_HASH);  
  printf("Version: %d.%d.%d\n", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH);
	if (is_hash_invalid(FW_GIT_HASH)) {
      printf("ERROR: Invalid firmware hash detected: %s\r\n", FW_GIT_HASH ? FW_GIT_HASH : "NULL");
  } else {
      printf("FW Hash: %s\r\n", FW_GIT_HASH);
  }
  FATFS_Init();

 // print_queue = NULL;
 // print_queue = xQueueCreate(PRINTER_MESSAGE_LEN, sizeof(printerMessage_t));
  
 // if(print_queue == NULL){
 //   while(1);
 // }

  lcd_printer_init();

	xTaskCreate(led_blinking_task, "blinky", BLINKY_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
	xTaskCreate(cdc_task, "cdc", CDC_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);
	vTaskStartScheduler();
}

void print_task(void* param) {
  (void) param;
  printerMessage_t msg;
  while(1){
    xQueueReceive(print_queue, &msg, portMAX_DELAY);
    // todo print msg
  }
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
