#ifndef _LCD_PRINTER_INIT
#define _LCD_PRINTER_INIT

#include <stdint.h>

__attribute__((format(printf, 3, 4)))
uint8_t lcd_print(uint8_t line, uint8_t offset, const char* format, ...);
void lcd_printer_init(void);

#endif