#include "status_service.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"

#define BLINKY_STACK_SIZE   configMINIMAL_STACK_SIZE

void led_blinking_task(void* param);

uint8_t status_service_init(void){
    xTaskCreate(led_blinking_task, "blinky", BLINKY_STACK_SIZE, NULL, 1, NULL);
    return 0;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void* param) {
  (void) param;
  static uint8_t led_state = 0;
  while (1) {
    SEGGER_SYSVIEW_PrintfHost("BlikTask started");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
//    led_state = 1 - led_state; // toggle
	  printf("blink %04d\n\r", led_state);
//    lcd_print(0, 1, "counter: %d", i++);
  }
}