#include "gpio_driver.h"

void gpio_led_go_change_state(uint8_t new_state){
    HAL_GPIO_WritePin (led_go_GPIO_Port, led_go_Pin, (new_state == GPIO_LED_OFF) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void gpio_led_status_change_state(uint8_t new_state){
    HAL_GPIO_WritePin (led_status_GPIO_Port, led_status_Pin, (new_state == GPIO_LED_OFF) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

int8_t gpio_led_mode_change_state(uint8_t led_mode, uint8_t new_state){
    uint8_t ret = 0;
    switch(led_mode){
        case LED_MODE_1: HAL_GPIO_WritePin (led_mode0_GPIO_Port, led_mode0_Pin, 
            (new_state == GPIO_LED_OFF) ? GPIO_PIN_RESET : GPIO_PIN_SET); ret = 0; break;
        case LED_MODE_2: HAL_GPIO_WritePin (led_mode1_GPIO_Port, led_mode1_Pin, 
            (new_state == GPIO_LED_OFF) ? GPIO_PIN_RESET : GPIO_PIN_SET); ret = 0; break;
        case LED_MODE_3: HAL_GPIO_WritePin (led_mode2_GPIO_Port, led_mode2_Pin, 
            (new_state == GPIO_LED_OFF) ? GPIO_PIN_RESET : GPIO_PIN_SET); ret = 0; break;
        case LED_MODE_4: HAL_GPIO_WritePin (led_mode3_GPIO_Port, led_mode3_Pin, 
            (new_state == GPIO_LED_OFF) ? GPIO_PIN_RESET : GPIO_PIN_SET); ret = 0; break;
        case LED_MODE_5: HAL_GPIO_WritePin (led_mode4_GPIO_Port, led_mode4_Pin, 
            (new_state == GPIO_LED_OFF) ? GPIO_PIN_RESET : GPIO_PIN_SET); ret = 0; break;
        case LED_MODE_6: HAL_GPIO_WritePin (led_mode5_GPIO_Port, led_mode5_Pin, 
            (new_state == GPIO_LED_OFF) ? GPIO_PIN_RESET : GPIO_PIN_SET); ret = 0; break;
        default: ret = -1; break;
    }
    return ret;
}

uint8_t is_button_mode_pressed(void){
    return HAL_GPIO_ReadPin(btn_mode_GPIO_Port, btn_mode_Pin) == 0;
}

uint8_t is_button_go_pressed(void){
    return HAL_GPIO_ReadPin(btn_go_GPIO_Port, btn_go_Pin) == 0;
}