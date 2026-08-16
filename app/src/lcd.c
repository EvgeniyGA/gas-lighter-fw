#include "lcd.h"

// Вспомогательная функция для отправки полубайта (4 бит)
static void LCD_SendNibble(LCD_HandleTypeDef* lcd, uint8_t nibble) {
    HAL_GPIO_WritePin(lcd->D4_Port, lcd->D4_Pin, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(lcd->D5_Port, lcd->D5_Pin, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(lcd->D6_Port, lcd->D6_Pin, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(lcd->D7_Port, lcd->D7_Pin, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    
    HAL_GPIO_WritePin(lcd->EN_Port, lcd->EN_Pin, GPIO_PIN_SET);
    // Небольшая задержка для установки строба (Enable)
    for(volatile int i = 0; i < 50; i++); 
    HAL_GPIO_WritePin(lcd->EN_Port, lcd->EN_Pin, GPIO_PIN_RESET);
    // Задержка выполнения команды (большинство команд требуют ~37 мкс)
    for(volatile int i = 0; i < 1000; i++); 
}

// Отправка байта (в 4-битном режиме отправляется двумя полубайтами)
static void LCD_SendByte(LCD_HandleTypeDef* lcd, uint8_t byte) {
    LCD_SendNibble(lcd, byte >> 4);       // Старший полубайт
    LCD_SendNibble(lcd, byte & 0x0F);     // Младший полубайт
}

void LCD_SendCommand(LCD_HandleTypeDef* lcd, uint8_t command) {
    HAL_GPIO_WritePin(lcd->RS_Port, lcd->RS_Pin, GPIO_PIN_RESET); // RS = 0 (команда)
    LCD_SendByte(lcd, command);
}

void LCD_SendData(LCD_HandleTypeDef* lcd, uint8_t data) {
    HAL_GPIO_WritePin(lcd->RS_Port, lcd->RS_Pin, GPIO_PIN_SET);   // RS = 1 (данные)
    LCD_SendByte(lcd, data);
}

void LCD_Init(LCD_HandleTypeDef* lcd) {
    // Ожидание стабилизации питания (минимум 15 мс)
    HAL_Delay(20);
    
    // Последовательность инициализации для 4-битного режима
    LCD_SendNibble(lcd, 0x03);
    HAL_Delay(5);
    
    LCD_SendNibble(lcd, 0x03);
    HAL_Delay(1);
    
    LCD_SendNibble(lcd, 0x03);
    HAL_Delay(1);
    
    LCD_SendNibble(lcd, 0x02); // Переход в 4-битный режим
    
    // Настройка функционала: 4-битная шина, 2+ строки, шрифт 5x8
    LCD_SendCommand(lcd, 0x28);
    
    // Включение дисплея: дисплей ВКЛ, курсор ВЫКЛ, мигание ВЫКЛ
    LCD_SendCommand(lcd, 0x0C);
    
    // Очистка дисплея
    LCD_SendCommand(lcd, 0x01);
    HAL_Delay(2); // Команда очистки требует больше времени (~1.52 мс)
    
    // Настройка режима ввода: инкремент адреса, без сдвига экрана
    LCD_SendCommand(lcd, 0x06);
}

void LCD_Clear(LCD_HandleTypeDef* lcd) {
    LCD_SendCommand(lcd, 0x01);
    HAL_Delay(2);
}

// Установка курсора.
// row: 0..3 (строки 1..4)
// col: 0..19 (символы 1..20)
void LCD_SetCursor(LCD_HandleTypeDef* lcd, uint8_t row, uint8_t col) {
    uint8_t address = 0;
    // Адресация DDRAM для 20x4 дисплеев (стандарт HD44780)
    switch (row) {
        case 0: address = 0x00; break;
        case 1: address = 0x40; break;
        case 2: address = 0x14; break;
        case 3: address = 0x54; break;
        default: address = 0x00; break;
    }
    address += col;
    // Бит 7 (0x80) устанавливается в 1 для команды Set DDRAM Address
    LCD_SendCommand(lcd, 0x80 | address);
}

void LCD_SendString(LCD_HandleTypeDef* lcd, const char* str) {
    while (*str) {
        LCD_SendData(lcd, (uint8_t)(*str));
        str++;
    }
}