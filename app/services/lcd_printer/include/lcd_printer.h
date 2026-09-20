#ifndef _LCD_PRINTER_INIT
#define _LCD_PRINTER_INIT

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum{
    LCD_PRINTER_LINE1 = 0,
    LCD_PRINTER_LINE2,
    LCD_PRINTER_LINE3,
    LCD_PRINTER_LINE4,
    LCD_PRINTER_LINES
}lcd_printer_lines_e;

typedef enum{
    LCD_PRINTER_OFFSET_ZERO = 0,
    LCD_PRINTER_OFFSET_HALF = 11, //check
    LCD_PRINTER_OFFSET_FULL_NEXT = 21
}lcd_printer_offset_e;

__attribute__((format(printf, 3, 4)))
uint8_t lcd_print(uint8_t line, uint8_t offset, const char* format, ...);
void lcd_printer_init(void);

#ifdef __cplusplus
}
#endif

#endif