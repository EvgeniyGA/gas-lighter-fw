#include <stdio.h>
#include "app.h"
#include "SEGGER_RTT.h"
#ifdef STM32F746xx
#include "stm32f7xx.h"
#elif defined STM32F407xx
#include "stm32f4xx.h"
#endif

extern UART_HandleTypeDef huart1;

int _write(int file, char *ptr, int len) {
    (void)file;
#ifdef PRINTF_RTT
    int written = SEGGER_RTT_Write(0, ptr, len);
    return written;
#elif PRINTF_UART
    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    if (status == HAL_OK) {
		return len; // Возвращаем количество успешно отправленных байт
	}
#endif
    return -1;
}

int debug_console_getchar(void) {
    uint8_t c;
#ifdef PRINTF_RTT
    if (SEGGER_RTT_Read(0, &c, 1) == 1) {
        return c;
    }
#elif PRINTF_UART
    if (HAL_UART_Receive(&huart1, &c, 1, 0) == HAL_OK) {
        return c;
    }
#endif
    return -1;
}

int _read(int file, char *ptr, int len) {
    if (file == 0) {
        int count = 0;
        while (count < len) {
            int c = debug_console_getchar();
            if (c == -1) {
                break;
            }
            *ptr++ = c;
            count++;
            if (c == '\n' || c == '\r') break; 
        }
        return count;
    }
    return -1;
}
