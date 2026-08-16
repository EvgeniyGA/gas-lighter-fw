#ifndef _LCD_PRINTER_INIT
#define _LCD_PRINTER_INIT

#include <stdint.h>

__attribute__((format(printf, 2, 3)))
uint8_t lcd_print(uint8_t line, const char* format, ...);

#endif