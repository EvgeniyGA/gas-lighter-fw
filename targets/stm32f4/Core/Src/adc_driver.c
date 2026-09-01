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

data_ready_callback_adc_t adc1_ready, adc2_ready;

int8_t adc_driver_register_callback(uint8_t adc_num, data_ready_callback_adc_t callback){
	switch(adc_num){
		case ADC_NUM_1: adc1_ready = callback; return 0;
		case ADC_NUM_2: adc2_ready = callback; return 0;
		default: break;
	}
	return -1;
}

uint8_t adc_driver_start(uint8_t adc_num, uint16_t* buff, uint16_t size){
	if(!size){
		return -1;
	}
	if(buff == NULL){
		return -1;
	}
	if(adc_num == ADC_NUM_1){
		HAL_ADC_Start_DMA (&hadc1, (uint32_t*) buff, size) ;
	}
	else if(adc_num == ADC_NUM_2){
		//HAL_TIM_Base_Start_IT(&htim8);//tim8 required for adc2
		HAL_ADC_Start_DMA (&hadc2, (uint32_t*) buff, size) ;
	}
	else{
		//toodo
	}
	return 0;
}

uint8_t adc_driver_stop(uint8_t adc_num){
	if(adc_num == ADC_NUM_1){
		HAL_ADC_Stop_DMA (&hadc1) ;
	}
	else if(adc_num == ADC_NUM_2){
		HAL_ADC_Stop_DMA (&hadc2) ;
	}
	else{
		//toodo
	}
	return 0;
}

inline void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	if(hadc->Instance == ADC1){
		adc1_ready(ADC_DATA_OFFSET_HALF);
	}
	else if(hadc->Instance == ADC2){
		adc2_ready(ADC_DATA_OFFSET_HALF);
	}
}

inline void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc){
	if(hadc->Instance == ADC1){
		adc1_ready(ADC_DATA_OFFSET_ZERO);
	}
	else if(hadc->Instance == ADC2){
		adc2_ready(ADC_DATA_OFFSET_ZERO);
	}
}
