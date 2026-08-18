#include "lcd_printer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

#define LCD_PRINTER_STACK_SIZE      configMINIMAL_STACK_SIZE * 2
#define LCD_MAX_LEN             (20)
#define LCD_PRINTER_BUF_LEN     (10)

typedef struct{
    uint8_t data[LCD_MAX_LEN];
    uint8_t data_len;
    uint8_t line;
}lcd_printer_msg_t;

StackType_t lcd_printer_stack[LCD_PRINTER_STACK_SIZE];
StaticTask_t lcd_printer_taskdef;
QueueHandle_t printerQueue;
uint8_t lcd_printer_queue_buf[LCD_PRINTER_BUF_LEN * sizeof(lcd_printer_msg_t)];
StaticQueue_t lcd_printer_queue;

void lcd_printer_task(void* param);

void lcd_printer_init(void){
    printerQueue = xQueueCreateStatic(LCD_PRINTER_BUF_LEN, sizeof(lcd_printer_msg_t), 
                                        lcd_printer_queue_buf, &lcd_printer_queue);
    xTaskCreateStatic(lcd_printer_task, "lcd_printer", LCD_PRINTER_STACK_SIZE, NULL, 
        configMAX_PRIORITIES - 4, lcd_printer_stack, &lcd_printer_taskdef);
}

void lcd_printer_task(void* param){
    lcd_printer_msg_t msg;
    while(1){
        if (xQueueReceive(printerQueue, &msg, portMAX_DELAY)) {
            printf("%.*s\n\r", msg.data_len, (char*)msg.data);
        }
        UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
        printf("LCD task stack free: %u words (%u bytes)\n\r", 
                   watermark, watermark * 4);
    }
}

uint8_t lcd_print(uint8_t line, const char* format, ...){
    lcd_printer_msg_t msg;
    msg.line = line;
    
    va_list args;
    va_start(args, format);
    int len = vsnprintf((char*)msg.data, LCD_MAX_LEN, format, args);
    va_end(args);
    
    if (len < 0) {
        return 1; 
    } else if (len >= LCD_MAX_LEN) {
        msg.data_len = LCD_MAX_LEN - 1; 
    } else {
        msg.data_len = (uint8_t)len;
    }
    
    if (xQueueSendToBack(printerQueue, &msg, 0) == pdPASS) {
        return 0;
    }

    return 1;
}