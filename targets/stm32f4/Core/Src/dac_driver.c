#include "dac_driver.h"

#include "dac.h"
#include "tim.h"

int dac_start(uint16_t* buf, uint32_t size, uint16_t tim_arr){
	__HAL_TIM_SET_AUTORELOAD(&htim8, tim_arr);
	HAL_TIM_Base_Start_IT(&htim8);
	if(HAL_DAC_Start_DMA(
			&hdac,
			DAC_CHANNEL_1,
			(uint32_t*) buf,
			size,
			DAC_ALIGN_12B_R) == HAL_OK){
		return 0;
	}
	return -1;
}

//inline void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
//  if (htim->Instance == htim8.Instance) {
//  }
//}

inline void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
}

inline void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
}
