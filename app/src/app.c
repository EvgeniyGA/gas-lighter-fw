#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include "task.h"

void BlinkTask(void *argument)
{
    for(;;)
    {
    	//dbgln("from task");
        //HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7); // Мигаем светодиодом (замени на свой пин)
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void init(void){
}

void setup(void){
	  xTaskCreate(
	      BlinkTask,        // Функция задачи
	      "Blink",          // Имя (для отладки, ни на что не влияет)
	      128,              // Размер стека в словах (128 * 4 = 512 байт)
	      NULL,             // Параметр, передаваемый в задачу (тут не нужен)
		  configMAX_PRIORITIES-1,                // Приоритет (1 — низкий, configMAX_PRIORITIES-1 — высокий)
	      NULL              // Handle задачи (если хочешь управлять ею позже)
	  );
	  vTaskStartScheduler();
}
