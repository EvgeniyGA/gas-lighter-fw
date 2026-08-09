/*
 * adc_driver.c
 *
 *  Created on: Apr 5, 2026
 *      Author: evgeny
 */
#include "adc_driver.h"
#include <stdint.h>

#include "adc.h"
#include "tim.h"
#include "wave_measure.h"

uint8_t adc_driver_start(uint16_t* buff, uint16_t size){
	if(!size){
		return -1;
	}
	if(buff == NULL){
		return -1;
	}
	HAL_TIM_Base_Start_IT(&htim8);
	HAL_ADC_Start_DMA (&hadc2, (uint32_t*) buff, size) ;
	return 0;
}

inline void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if(hadc->Instance == ADC2){
		wave_measure_adc_callback () ;
	}
}

