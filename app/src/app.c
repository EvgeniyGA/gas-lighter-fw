#include <FreeRTOS.h>
#include "task.h"
#include "SEGGER_RTT.h"

#define dbg(s)                        SEGGER_RTT_WriteString( 0, s )//; HAL_Delay(1)
#define dbgln(s)                      dbg( s "\n" )
#define dbgf( format, ... )           SEGGER_RTT_printf( 0, ( const char * ) ( format ), ##__VA_ARGS__ ); HAL_Delay(1)

void BlinkTask(void *argument)
{
    for(;;)
    {
        //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
        vTaskDelay(pdMS_TO_TICKS(500));
        dbgln("blink from freertos!!!!");
    }
}

void init(void){
	SEGGER_RTT_ConfigUpBuffer( 0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM );
    SEGGER_RTT_WriteString( 0, "SEGGER Real-Time-Terminal Sample\n" );
}

void setup(void){
	  xTaskCreate(
	      BlinkTask,
	      "Blink",
	      128,
	      NULL,
		  configMAX_PRIORITIES-1,
	      NULL
	  );
	  vTaskStartScheduler();
}
