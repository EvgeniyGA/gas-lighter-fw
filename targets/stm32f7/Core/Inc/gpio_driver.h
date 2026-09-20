#ifndef _GPIO_DRIVER_H
#define _GPIO_DRIVER_H

#include "main.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    GPIO_LED_OFF = 0,
    GPIO_LED_ON
}led_state_e;

typedef enum {
    LED_MODE_1 = 0,
    LED_MODE_2,
    LED_MODE_3,
    LED_MODE_4, 
    LED_MODE_5,
    LED_MODE_6,
    LED_MODE_COUNT
}mode_number_e;

int8_t gpio_led_mode_change_state(uint8_t led_mode, uint8_t new_state);
uint8_t is_button_mode_pressed(void);
uint8_t is_button_go_pressed(void);
void gpio_led_go_change_state(uint8_t new_state);
void gpio_led_status_change_state(uint8_t new_state);

#ifdef __cplusplus
}
#endif

#endif