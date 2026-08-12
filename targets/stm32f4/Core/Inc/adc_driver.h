/*
 * adc_driver.h
 *
 *  Created on: Apr 5, 2026
 *      Author: evgeny
 */

#ifndef INC_ADC_DRIVER_H_
#define INC_ADC_DRIVER_H_

#include <stdint.h>

typedef enum{
    ADC_NUM_1 = 1,
    ADC_NUM_2,
    ADC_NUM_3
}adc_numbers_e;

uint8_t adc_driver_start(uint8_t adc_num, uint16_t* buff, uint16_t size);
uint8_t adc_driver_stop(uint8_t adc_num);

#endif /* INC_ADC_DRIVER_H_ */
