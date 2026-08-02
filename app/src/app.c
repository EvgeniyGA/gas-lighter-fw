#include <stdio.h>
#include <ctype.h>
#include <FreeRTOS.h>
#include "task.h"
#include "SEGGER_RTT.h"
#include "tusb.h"
#include "fatfs.h"
#include "usb_service.h"
#include "version.h"

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

void led_blinking_task(void* param);


void init(void){
	SEGGER_RTT_ConfigUpBuffer( 0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM );
	SEGGER_RTT_WriteString( 0, "SEGGER Real-Time-Terminal Started\n" );
}

void setup(void){
	printf("Firmware version: %s\n", FW_VERSION_STR);
    printf("Build: %s %s (git: %s)\n", FW_BUILD_DATE, FW_BUILD_TIME, FW_GIT_HASH);
    
    printf("Version: %d.%d.%d\n", FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH);
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

  while (1) {
    // Blink every interval ms
    vTaskDelay(blink_interval_ms / portTICK_PERIOD_MS);
    //board_led_write(led_state);
    led_state = 1 - led_state; // toggle
	static uint32_t i;
	printf("blink %04d\n\r", i++);
  }
}
